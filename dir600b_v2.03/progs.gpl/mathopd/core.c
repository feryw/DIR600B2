/* vi: set sw=4 ts=4: */
/*
 *   Copyright 1996 - 2004 Michiel Boland.
 *   All rights reserved.
 *
 *   Redistribution and use in source and binary forms, with or
 *   without modification, are permitted provided that the following
 *   conditions are met:
 *
 *   1. Redistributions of source code must retain the above
 *      copyright notice, this list of conditions and the following
 *      disclaimer.
 *
 *   2. Redistributions in binary form must reproduce the above
 *      copyright notice, this list of conditions and the following
 *      disclaimer in the documentation and/or other materials
 *      provided with the distribution.
 *
 *   3. The name of the author may not be used to endorse or promote
 *      products derived from this software without specific prior
 *      written permission.
 *
 *   THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY
 *   EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 *   THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 *   PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR
 *   BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 *   EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 *   TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *   ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 *   LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 *   IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 *   THE POSSIBILITY OF SUCH DAMAGE.
 */

/* Les trois soers aveugles */

static const char rcsid[] = "$Id: core.c,v 1.1.1.1 2005/05/19 10:53:06 r01122 Exp $";

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/sysinfo.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <linux/ip.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <time.h>
#ifndef POLL_EMULATION
#include <poll.h>
#else
#include "poll-emul.h"
#endif
#include <stdlib.h>
#include <string.h>
#include "mathopd.h"
/*------------------------------*/
/* Modified by Paul Liu 20061212 */
/* Add a header file for unix socket */
#include "unixsock.h"
/*------------------------------*/
#include "rgdb_getset.h"


#ifdef NEED_SOCKLEN_T
typedef int socklen_t;
#endif

struct statistics stats;
time_t startuptime;
time_t current_time;
/*------------------------------*/
/* Added/Modified by Paul Liu 20061220 */
/* Modified time check from time(0) to uptime */
long current_uptime;
/*------------------------------*/
struct pollfd *pollfds;
struct connection *connection_array;

static struct connection_list free_connections;
static struct connection_list waiting_connections;
static struct connection_list reading_connections;
static struct connection_list writing_connections;
static struct connection_list forked_connections;

/*------------------------------*/
/* Modified by Paul Liu 20060324 */
/* Add a struct for multipart upload */
extern upload upload_file;
extern void check_upgrad(void);

/* Modified by Paul Liu 20061212 */
/* Add a struct for command server */
struct _cmd_serrver
{
	usock_handle usock;
	int pollno;
}cmd_server={NULL, 0};

#define CMD_SOCK "/var/run/mathopd_usock"

int enter_csection=0;
/*------------------------------*/

static void c_unlink(struct connection *c, struct connection_list *l)
{
	struct connection *p, *n;

	p = c->prev;
	n = c->next;
	if (p)
		p->next = n;
	if (n)
		n->prev = p;
	c->prev = 0;
	c->next = 0;
	if (c == l->head)
		l->head = n;
	if (c == l->tail)
		l->tail = p;
}

static void c_link(struct connection *c, struct connection_list *l)
{
	struct connection *n;

	n = l->head;
	c->prev = 0;
	c->next = n;
	if (n == 0)
		l->tail = c;
	else
		n->prev = c;
	l->head = c;
}

static struct connection_list *clistp(enum connection_state state)
{
	switch (state) {
	default:
		return 0;
	case HC_FREE:
		return &free_connections;
	case HC_WAITING:
		return &waiting_connections;
	case HC_READING:
		return &reading_connections;
	case HC_WRITING:
		return &writing_connections;
	case HC_FORKED:
		return &forked_connections;
	}
}

void set_connection_state(struct connection *c, enum connection_state state)
{
	enum connection_state oldstate;
	struct connection_list *o, *n;

	oldstate = c->connection_state;
	if (state == oldstate)
		return;
	o = clistp(oldstate);
	n = clistp(state);
	if (o)
		c_unlink(c, o);
	c->connection_state = state;
	if (n)
		c_link(c, n);
}

static void init_connection(struct connection *cn)
{
	cn->header_input.state = 0;
	cn->output.start = cn->output.end = cn->output.floor;
	init_request(cn->r);
	cn->keepalive = 0;
	cn->nread = 0;
	cn->nwritten = 0;
	cn->left = 0;
	cn->havefile = 0;
	gettimeofday(&cn->itv, 0);
}

static int scan_request(struct connection *cn);

int reinit_connection(struct connection *cn)
{
	char *s;

	++stats.nrequests;
	log_request(cn->r);
	cn->logged = 1;
	if (cn->rfd != -1) {
		close(cn->rfd);
		cn->rfd = -1;
	}
	init_connection(cn);
	set_connection_state(cn, HC_WAITING);
	s = cn->header_input.middle;
	if (s == cn->header_input.end) {
		cn->header_input.start = cn->header_input.middle = cn->header_input.end = cn->header_input.floor;
		return 0;
	}
	++stats.pipelined_requests;
	cn->header_input.start = s;
	return scan_request(cn);
}

void close_connection(struct connection *cn)
{
	/*------------------------------*/
	/* Added by Paul Liu 20040615 */
	/* Free memory for multipart upload error */
	if(upload_file.flag==0 && upload_file.data)
	{
		free(upload_file.data);
		memset(&upload_file, 0, sizeof(upload_file));
	}
	/*------------------------------*/

	if (cn->nread || cn->nwritten || cn->logged == 0) {
		++stats.nrequests;
		log_request(cn->r);
	}
	--stats.nconnections;
	/*------------------------------*/
	/* Modified by Paul Liu 20061003 */
	/* Add support for UDP type server */
	if(!cn->s->udp)
	{
		if (debug)
			log_d("close_connection: %d", cn->fd);
		close(cn->fd);
	}
	/*------------------------------*/
	if (cn->rfd != -1) {
		close(cn->rfd);
		cn->rfd = -1;
	}
	set_connection_state(cn, HC_FREE);
}

static void close_servers(void)
{
	struct server *s;

	s = servers;
	while (s) {
		if (s->fd != -1) {
			close(s->fd);
			s->fd = -1;
		}
		s = s->next;
	}
}

static void close_connections(void)
{
	struct connection *c, *n;

	c = waiting_connections.head;
	while (c) {
		n = c->next;
		close_connection(c);
		c = n;
	}
	c = reading_connections.head;
	while (c) {
		n = c->next;
		close_connection(c);
		c = n;
	}
	c = writing_connections.head;
	while (c) {
		n = c->next;
		close_connection(c);
		c = n;
	}
	c = forked_connections.head;
	while (c) {
		n = c->next;
		close_connection(c);
		c = n;
	}
}

static struct connection *find_connection(void)
{
	if (free_connections.head)
		return free_connections.head;
	if (tuning.clobber)
		return waiting_connections.tail;
	return 0;
}

#ifdef ELBOX_USE_IPV6
#ifdef SOCK_DGRAM_SUPPORT
static struct connection * find_udp_connection(int fd)
{
	struct connection * cn;

	for (cn=waiting_connections.head; cn; cn=cn->next)
		if (cn->fd == fd) return cn;
	for (cn=reading_connections.head; cn; cn=cn->next)
		if (cn->fd == fd) return cn;
	for (cn=writing_connections.head; cn; cn=cn->next)
		if (cn->fd == fd) return cn;
	for (cn=forked_connections.head; cn; cn=cn->next)
		if (cn->fd == fd) return cn;
	return NULL;
}

static int accept_udp_connection(struct server *s)
{
	socklen_t l;
	int fd, rv;
	struct connection *cn;

	do
	{
		cn = find_udp_connection(s->fd);
		if (cn != 0) break;
		cn = find_connection();
		if (cn == 0) return 0;

		/* We don't need to accept a datagram connection. */
		fd = s->fd;
		++stats.accepted_connections;
		if (debug) log_d("%s: %d", __func__,fd);
		fcntl(fd, F_SETFD, FD_CLOEXEC);
		fcntl(fd, F_SETFL, O_NONBLOCK);

		if (cn->connection_state != HC_FREE)
		{
			if (debug) log_d("%s: clobbering connection to %s[%s]",__func__,
								cn->peer.ap_address, cn->peer.ap_port);
			close_connection(cn);
		}

		/* The peer info will be filled in when the data is receiving. */
		/* Fill in the local information only. */
		l = sizeof cn->sa_local;
		if (getsockname(fd, (struct sockaddr *) &cn->sa_local, &l) == -1)
		{
			lerror("getsockname");
			break;
		}

		rv = getnameinfo((struct sockaddr *) &cn->sa_local, l,
				cn->sock.ap_address, sizeof cn->sock.ap_address,
				cn->sock.ap_port, sizeof cn->sock.ap_port,
				NI_NUMERICHOST | NI_NUMERICSERV);
		if (rv)
		{
			log_d("%s: getnameinfo failed for sock: %s",__func__,gai_strerror(rv));
			break;
		}
		/* setup the connection */
		cn->s		= s;
		cn->fd		= fd;
		cn->rfd		= -1;
		cn->t		= current_time;
		cn->pollno	= -1;

		++stats.nconnections;
		if (stats.nconnections > stats.maxconnections) stats.maxconnections = stats.nconnections;
		init_connection(cn);
		cn->logged = 0;
		cn->header_input.start = cn->header_input.middle = cn->header_input.end = cn->header_input.floor;
		set_connection_state(cn, HC_WAITING);
	} while (0);
	return 0;
}
#endif
#endif

static int accept_connection(struct server *s)
{
#ifdef ELBOX_USE_IPV6	
	struct sockaddr_storage sa_remote, sa_local;
	int rv, mss;
#else	
	struct sockaddr_in sa_remote, sa_local;
#endif
	socklen_t l;
	int fd;
	struct connection *cn;

#ifdef ELBOX_USE_IPV6
#ifdef SOCK_DGRAM_SUPPORT
	if (s->socktype == SOCK_DGRAM) return accept_udp_connection(s);
#endif
#endif

	do {
		cn = find_connection();
		if (cn == 0)
			return 0;
		l = sizeof sa_remote;
#ifndef ELBOX_USE_IPV6	
		/*------------------------------*/
		/* Modified by Paul Liu 20061003 */
		/* Add support for UDP type server */
		if(s->udp)
			fd=s->fd;
		else
			fd = accept(s->fd, (struct sockaddr *) &sa_remote, &l);
#else		
			fd = accept(s->fd, (struct sockaddr *) &sa_remote, &l);
		/*------------------------------*/
#endif		
		if (fd == -1)
			switch (errno) {
			case EAGAIN:
				return 0;
#ifdef ECONNABORTED
			case ECONNABORTED:
#endif
#ifdef EPROTO
			case EPROTO:
#endif
				lerror("accept");
				continue;
			default:
				lerror("accept");
				return -1;
			}
		++stats.accepted_connections;
		if (debug)
			log_d("accept_connection: %d", fd);
		fcntl(fd, F_SETFD, FD_CLOEXEC);
		fcntl(fd, F_SETFL, O_NONBLOCK);
#ifndef ELBOX_USE_IPV6		
		l = sizeof sa_local;
		if (getsockname(fd, (struct sockaddr *) &sa_local, &l) == -1) {
			lerror("getsockname");
			close(fd);
			break;
		}
#endif		
		if (cn->connection_state != HC_FREE) {
			if (debug)
#ifdef ELBOX_USE_IPV6	
				log_d("clobbering connection to %s[%s]", cn->peer.ap_address, cn->peer.ap_port);
#else			
				log_d("clobbering connection to %s[%hu]", inet_ntoa(cn->peer.sin_addr), ntohs(cn->peer.sin_port));
#endif
			close_connection(cn);
		}
		
#ifdef ELBOX_USE_IPV6
		rv = getnameinfo((struct sockaddr *) &sa_remote, l, cn->peer.ap_address, sizeof cn->peer.ap_address, cn->peer.ap_port, sizeof cn->peer.ap_port, NI_NUMERICHOST | NI_NUMERICSERV);
		if (rv) {
			log_d("accept_connection: getnameinfo failed for peer: %s", gai_strerror(rv));
			close(fd);
			break;
		}
		l = sizeof sa_local;
		if (getsockname(fd, (struct sockaddr *) &sa_local, &l) == -1) {
			lerror("getsockname");
			close(fd);
			break;
		}
		rv = getnameinfo((struct sockaddr *) &sa_local, l, cn->sock.ap_address, sizeof cn->sock.ap_address, cn->sock.ap_port, sizeof cn->sock.ap_port, NI_NUMERICHOST | NI_NUMERICSERV);
		if (rv) {
			log_d("accept_connection: getnameinfo failed for sock: %s", gai_strerror(rv));
			close(fd);
			break;
		}
		l = sizeof mss;
		if (getsockopt(fd, IPPROTO_TCP, TCP_MAXSEG, &mss, &l) == -1) {
			lerror("getsockopt");
			close(fd);
			break;
		}
#endif		
		cn->s = s;
		cn->fd = fd;
		cn->rfd = -1;
#ifndef ELBOX_USE_IPV6	
		/*------------------------------*/
		/* Modified by Paul Liu 20061003 */
		/* Add support for UDP type server */
		/* We can not get remote while udp server, we get it when we receive data */
		if(!s->udp)
			cn->peer = sa_remote;
		/*------------------------------*/
		cn->sock = sa_local;
#endif		
		cn->t = current_uptime;
		cn->pollno = -1;
		++stats.nconnections;
		if (stats.nconnections > stats.maxconnections)
			stats.maxconnections = stats.nconnections;
#ifdef ELBOX_USE_IPV6				
		//pool_adjust(&cn->output, mss);	
#endif		
		init_connection(cn);
		cn->logged = 0;
		cn->header_input.start = cn->header_input.middle = cn->header_input.end = cn->header_input.floor;
		set_connection_state(cn, HC_WAITING);
	/*------------------------------*/
	/* Modified by Paul Liu 20061003 */
	/* Add support for UDP type server */
	/* Do not accept multi when udp server */
	} 
#ifdef ELBOX_USE_IPV6	
	while (tuning.accept_multi);
#else
	while (tuning.accept_multi && !s->udp);
#endif
	/*------------------------------*/
	return 0;
}

#if ! (defined LINUX_SENDFILE || defined FREEBSD_SENDFILE)
static int fill_connection(struct connection *cn)
{
	struct pool *p;
	int poolleft, n, m;
	intmax_t fileleft;

	if (cn->rfd == -1)
		return 0;
	p = &cn->output;
	poolleft = p->ceiling - p->end;
	fileleft = cn->left;
	n = fileleft > poolleft ? poolleft : (int) fileleft;
	if (n <= 0)
		return 0;
	cn->left -= n;
	m = read(cn->rfd, p->end, n);
	if (debug)
		log_d("fill_connection: %d %zd %d %d", cn->rfd, p->end - p->floor, n, m);
	if (m != n) {
		if (m == -1)
			lerror("read");
		else
			log_d("premature end of file %s", cn->r->path_translated);
		return -1;
	}
	p->end += n;
	cn->file_offset += n;
	return n;
}
#endif

static void end_response(struct connection *cn)
{
#if defined LINUX_SENDFILE || defined FREEBSD_SENDFILE
	if (cn->havefile)
		if (set_nopush(cn->fd, 0) == -1) {
			if (debug)
				lerror("set_nopush");
			close_connection(cn);
			return;
		}
#endif
	if (cn->keepalive)
		reinit_connection(cn);
	else
		close_connection(cn);
}

static void write_connection(struct connection *cn)
{
	struct pool *p;
	int m, n;

	p = &cn->output;
	do {
		n = p->end - p->start;
		if (n == 0) {
#if defined LINUX_SENDFILE || defined FREEBSD_SENDFILE
			if (sendfile_connection(cn) == -1) {
				close_connection(cn);
				return;
			}
			if (cn->left == 0)
		       		end_response(cn);
			return;
#else
			p->start = p->end = p->floor;
			n = fill_connection(cn);
			if (n == -1) {
				close_connection(cn);
				return;
			}
			if (n == 0) {
				end_response(cn);
				return;
			}
#endif
		}
		/*------------------------------*/
		/* Modified by Paul Liu 20061003 */
		/* Add support for UDP type server */
		if(cn->s->udp)
			m = sendto(cn->fd, p->start, n, 0, (struct sockaddr *) &cn->peer, sizeof(cn->peer));
		else
			m = write(cn->fd, p->start, n);
		/*------------------------------*/
		if (debug)
			log_d("write_connection: %d %zd %d %d", cn->fd, p->start - p->floor, n, m);
		if (m == -1) {
			if (errno == EAGAIN)
				return;
			if (debug)
				lerror("write");
			close_connection(cn);
			return;
		}
		cn->t = current_uptime;
		cn->nwritten += m;
		p->start += m;
	} while (n == m);
}

static int begin_response(struct connection *cn)
{
	if (cn->rfd == -1) {
		cn->left = 0;
		return 0;
	}
	cn->havefile = 1;
	cn->left = cn->r->content_length;
#if defined LINUX_SENDFILE || defined FREEBSD_SENDFILE
	if (set_nopush(cn->fd, 1) == -1) {
		if (debug)
			lerror("set_nopush");
		return -1;
	}
	return 0;
#else
	if (cn->file_offset)
		if (lseek(cn->rfd, cn->file_offset, SEEK_SET) == -1) {
			lerror("lseek");
			return -1;
		}
	return fill_connection(cn);
#endif
}

/*--------------------------------------*/
/* Added by Paul Liu 20040514				*/
/* Locate a substring in specify size	*/
static char *strnstr(char *pbuf, char *pat, int size)
{
	int  i, j;
	int  patlen=strlen(pat);
	char *pMatch;

	if (!pbuf || !pat || size <= 0 || patlen == 0) return NULL;

	for (i=0; i<size; i++)
	{
		if (pbuf[i]==pat[0])
		{
			pMatch=pbuf+i;
			for (j=1; j<patlen && j+i<size; j++)
			{
				if (pbuf[i+j]!=pat[j]) break;
			}
			if (j==patlen) return pMatch;
		}
	}
	return NULL;
}

/*------------------------------*/
/* Added by Paul Liu 20040426		*/
/* Check for multipart upload	*/
static int scan_multipart(struct connection *cn)
{
	struct request *r=cn->r;
	char   *pStart, *pEnd, *pBufEnd;
	int    FileSect=0;

	if (upload_file.data && upload_file.length && upload_file.length<tuning.max_uploadsize)
	{
		pBufEnd = upload_file.data+upload_file.length;
		pStart = upload_file.data;
		while ((pEnd = strnstr(pStart, r->multi_boundary, pBufEnd-pStart)))
		{
			if (FileSect)
			{
				upload_file.file_length = (pEnd-upload_file.file_ptr)-4;
				FileSect=0;
			}

			pStart = pEnd + strlen(r->multi_boundary);
			if ((pEnd=strnstr(pStart, "Content-Disposition", pBufEnd-pStart)))
			{
				pStart = pEnd+19;	/* 19=strlen("Content-Disposition") */
				if ((pEnd=strnstr(pStart, "filename=", pBufEnd-pStart)))
				{
					pStart = pEnd+10;	/* 10=strlen("filename=\"") */
					if ((pEnd = strnstr(pStart, "\"", pBufEnd-pStart)))
					{
						upload_file.file_name = pStart;
						pStart = pEnd+1;
						*pEnd = 0;
						if ((pEnd = strnstr(pStart, "\r\n\r\n", pBufEnd-pStart)))
						{
							pStart = pEnd+4;
							upload_file.file_ptr = pStart;
							FileSect = 1;
						}
					}
				}
				else if ((pEnd = strnstr(pStart, "form-data; name=", pBufEnd-pStart)))
				{
					/*
					 * To Do
					 */
				}
			}
		}
	}

	if (process_request(r) == -1 && cn->connection_state != HC_FORKED)
	{
		close_connection(cn);
		return -1;
	}

	if(!cn->r->status)
		set_connection_state(cn, HC_WRITING);

	return 0;
}

/* Read multipart upload */
static int read_multipart(struct connection *cn)
{
	struct request *r=cn->r;
	size_t bytestoread, nr, len=0;

	if (!upload_file.data)
	{
		//-----shutdown some service for extracting more memory
		system("/etc/scripts/misc/preupgrade.sh");
		len=r->content_length=atoi(r->in_content_length);

		if(r->content_length > tuning.max_uploadsize)
			len = tuning.max_uploadsize;

		if (NULL==(upload_file.data=(char *)malloc(len*sizeof(char))))
		{
			log_d("can not alloc input buffer");
			close_connection(cn);
			return -1;
		}

		/* Move content date from cn->header_input, if any */
		bytestoread = cn->header_input.end-cn->header_input.middle;
		if (bytestoread && bytestoread<=len)
		{
			memcpy(upload_file.data, cn->header_input.middle, bytestoread);
			upload_file.length=bytestoread;
			if(bytestoread>=r->content_length)
				return scan_multipart(cn);
		}
	}

	if(r->content_length > tuning.max_uploadsize)
		nr = read(cn->fd, upload_file.data, tuning.max_uploadsize);
	else
	{
		bytestoread = r->content_length-upload_file.length;
		nr = read(cn->fd, upload_file.data+upload_file.length, bytestoread);
	}

	if (nr == -1)
	{
		if (errno == EAGAIN) return -1;
		if (debug) lerror("read multipart");
		close_connection(cn);
		return -1;
	}
	if (nr == 0)
	{
		close_connection(cn);
		return -1;
	}
	upload_file.length += nr;

	if (upload_file.length >= r->content_length) return scan_multipart(cn);
	return -1;
}
/*------------------------------*/

static int read_connection(struct connection *cn)
{
	size_t bytestoread, bytestomove, offset;
	struct sockaddr_in sa_remote;
	ssize_t nr;

	bytestoread = cn->header_input.ceiling - cn->header_input.end;
	if (bytestoread == 0) {
		offset = cn->header_input.start - cn->header_input.floor;
		if (offset == 0) {
			log_d("input buffer full");
			close_connection(cn);
			return -1;
		}
		bytestomove = cn->header_input.end - cn->header_input.start;
		memmove(cn->header_input.floor, cn->header_input.start, bytestomove);
		cn->header_input.start -= offset;
		cn->header_input.middle -= offset;
		cn->header_input.end -= offset;
		bytestoread = cn->header_input.ceiling - cn->header_input.end;
	}
	/*------------------------------*/
	/* Modified by Paul Liu 20061003 */
	/* Add support for UDP type server */
	/* Get remote address if udp server */
	if(cn->s->udp)
	{
		socklen_t l=sizeof(sa_remote);
		nr = recvfrom(cn->fd, cn->header_input.end, bytestoread, 0, (struct sockaddr *) &sa_remote, &l);
#ifdef ELBOX_USE_IPV6
		//if(nr) 	cn->peer = sa_remote;
		if (nr)
		{
			/* fill in the peer information */
			int rv = getnameinfo((struct sockaddr *)&cn->sa_remote, l,
						cn->peer.ap_address, sizeof(cn->peer.ap_address),
						cn->peer.ap_port, sizeof(cn->peer.ap_port),
						NI_NUMERICHOST | NI_NUMERICSERV);
			if (rv)
			{
				log_d("read_connection: unable to get peer info: %s", gai_strerror(rv));
				close_connection(cn);
				return -1;
			}
		}
#else
		if(nr) 	cn->peer = sa_remote;
#endif
	}
	else
		nr = read(cn->fd, cn->header_input.end, bytestoread);

	/* Added by Paul Liu 20050222 */
	/* Add a termination for read contents, avoid getting previous inputs at parse stage*/
	*(cn->header_input.end+nr)=0;
	/*------------------------------*/
	if (debug)
		log_d("read_connection: %d %zd %zu %zd", cn->fd, cn->header_input.end - cn->header_input.floor, bytestoread, nr);
	if (nr == -1) {
		if (errno == EAGAIN)
			return 0;
		if (debug)
			lerror("read");
		close_connection(cn);
		return -1;
	}
	if (nr == 0) {
		close_connection(cn);
		return -1;
	}
	cn->nread += nr;
	cn->header_input.end += nr;
	cn->t = current_uptime;
	return 0;
}

static int scan_request(struct connection *cn)
{
	char *s;
	int c, state=0;

	s = cn->header_input.middle;
	state = cn->header_input.state;
	while (state != 8 && s < cn->header_input.end)
	{
		c = *s++;
		if (c == 0)
		{
			log_d("read_connection: NUL in headers");
			close_connection(cn);
			return -1;
		}
		switch (state)
		{
		case 0:
			switch (c)
			{
			default:	state = 1; break;

			case '\r':
			case '\n':	break;

			case ' ':
			case '\t':	state = 2; break;
			}
			if (state)
			{
				gettimeofday(&cn->itv, 0);
				set_connection_state(cn, HC_READING);
			}
			break;

		case 1:
			switch (c)
			{
			default:	break;

			case ' ':
			case '\t':	state = 2; break;
			case '\r':	state = 3; break;
			case '\n':	state = 8; break;
			}
			break;
		case 2:
			switch (c)
			{
			case 'H':	state = 4; break;
			default:	state = 1; break;

			case ' ':
			case '\t':	break;

			case '\r':	state = 3; break;
			case '\n':	state = 8; break;
			}
			break;
		case 3:
			switch (c)
			{
			case '\n':	state = 8; break;
			default:	state = 1; break;

			case ' ':
			case '\t':	state = 2; break;

			case '\r':	break;
			}
			break;
		case 4:
			switch (c)
			{
			case '\r':	state = 5; break;
			case '\n':	state = 6; break;
			}
			break;
		case 5:
			switch (c)
			{
			default:	state = 4;
			case '\r':	break;
			case '\n':	state = 6; break;
			}
			break;
		case 6:
			switch (c)
			{
			case '\r':	state = 7; break;
			case '\n':	state = 8; break;
			default:	state = 4; break;
			}
			break;
		case 7:
			switch (c)
			{
			default:	state = 4;
			case '\r':	break;
			case '\n':	state = 8; break;
			}
			break;
		}
	}

	cn->header_input.state = state;
    cn->header_input.middle = s;
	if (state == 8)
	{
		if (process_request(cn->r) == -1)
		{
			/*------------------------------*/
			/* Modified by Paul Liu 20040426 */
			/* Check for multipart upload */
			/* Modified by Paul Liu 20050729*/
			/* Check for contents in more than one packages */
			if (!cn->r->in_mblen && !cn->r->multi_boundary && cn->connection_state != HC_FORKED)
			{
				close_connection(cn);
				return -1;
			}
			/*------------------------------*/
			return 0;
		}
		if (begin_response(cn) == -1)
		{
			close_connection(cn);
			return -1;
		}
		set_connection_state(cn, HC_WRITING);
	}
	return 0;
}

static int setup_server_pollfds(int n)
{
	struct server *s;

	/*------------------------------*/
	/* Modified by Paul Liu 20061212 */
	/* Do not receive anything while entering critical sesction */
	if(!enter_csection)
	{
		s = servers;
		while (s)
		{
			if (s->fd != -1)
			{
				pollfds[n].events = POLLIN;
				pollfds[n].fd = s->fd;
				s->pollno = n++;
			}
			else
				s->pollno = -1;
			s = s->next;
		}
	}

	/* Added by Paul Liu 20061212 */
	/* Add pollfds for unix sock command server */
	if(cmd_server.usock && usock_fd(cmd_server.usock))
	{
		pollfds[n].events = POLLIN;
		pollfds[n].fd = usock_fd(cmd_server.usock);
		cmd_server.pollno=n++;
	}
	/*------------------------------*/
	return n;
}

static int setup_connection_pollfds(int n)
{
	struct connection *cn;

	/*------------------------------*/
	/* Modified by Paul Liu 20061212 */
	/* Do not receive anything while entering critical sesction */
	if(!enter_csection)
	{
		cn = waiting_connections.head;
		while (cn)
		{
			pollfds[n].fd = cn->fd;
			pollfds[n].events = POLLIN;
			cn->pollno = n++;
			cn = cn->next;
		}
		cn = reading_connections.head;
		while (cn)
		{
			pollfds[n].fd = cn->fd;
			pollfds[n].events = POLLIN;
			cn->pollno = n++;
			cn = cn->next;
		}
	}
	/*------------------------------*/
	cn = writing_connections.head;
	while (cn) {
		pollfds[n].fd = cn->fd;
		pollfds[n].events = POLLOUT;
		cn->pollno = n++;
		cn = cn->next;
	}
	return n;
}

/*------------------------------*/
/* Added by Paul Liu 20061212 */
/* Add support for unix sock command server */
/* Parse command */
static void parse_command(char *pCmd)
{
	if(!strcasecmp(pCmd, "enter_csection"))
	{
		log_d("received command: entering critical section");
		enter_csection=1;
	}
	else if(!strcasecmp(pCmd, "leave_csection"))
	{
		log_d("received command: leaving critical section");
		enter_csection=0;
	}
}

/* poll commands */
static void run_command(void)
{
	if (pollfds[cmd_server.pollno].revents & POLLIN)
	{
		int len;
		char buf[512] = {0};
		if((len = usock_recv(cmd_server.usock, buf, sizeof(buf), 0)) > 0)
			parse_command(buf);
		else
			log_d("warning: received a error command...");
	}
}
/*------------------------------*/

static int run_servers(void)
{
	struct server *s;

	s = servers;
	while (s) {
		if (s->pollno != -1)
			if (pollfds[s->pollno].revents & POLLIN)
				if (accept_connection(s) == -1)
					return -1;
		s = s->next;
	}
	return 0;
}

static void run_rconnection(struct connection *cn)
{
	int n;
	short r;

	n = cn->pollno;
	if (n == -1)
		return;
	r = pollfds[n].revents;
	if (r & POLLERR || r == POLLHUP)
	{
		close_connection(cn);
		return;
	}

	if (r & POLLIN)
	{
		/*------------------------------*/
		/* Modified by Paul Liu 20040426 */
		/* Check for multipart upload */
		if (cn->r->multi_boundary)
		{
			if (read_multipart(cn) == -1) return;
		}
		else if (read_connection(cn) == -1 || scan_request(cn) == -1)
		{
			return;
		}
		else if (cn->r->multi_boundary && read_multipart(cn) == -1)
		{
			//-----Check boundary again...maybe we got it from read_connection...
			return;
		}
		/*------------------------------*/

		r &= ~POLLIN;
		if (cn->connection_state == HC_WRITING) r |= POLLOUT;
		pollfds[n].revents = r;
	}
}

static void run_wconnection(struct connection *cn)
{
	int n;
	short r;

	n = cn->pollno;
	if (n == -1)
		return;
	r = pollfds[n].revents;
	if (r & POLLERR || r == POLLHUP) {
		close_connection(cn);
		return;
	}
	if (r & POLLOUT)
		write_connection(cn);
}

static void run_connections(void)
{
	struct connection *c, *n;

	/* Modified by Paul Liu 20061212 */
	/* Do not receive anything while entering critical sesction */
	if(!enter_csection)
	{
		c = waiting_connections.head;
		while (c)
		{
			n = c->next;
			run_rconnection(c);
			c = n;
		}
		c = reading_connections.head;
		while (c)
		{
			n = c->next;
			run_rconnection(c);
			c = n;
		}
	}
	/*------------------------------*/
	c = writing_connections.head;
	while (c) {
		n = c->next;
		run_wconnection(c);
		c = n;
	}
	c = forked_connections.head;
	while (c) {
		n = c->next;
		pipe_run(c);
		c = n;
	}
}

static void timeout_connections(struct connection *c, time_t t, const char *what)
{
	struct connection *n;

	while (c) {
		n = c->next;
		if (current_uptime >= c->t + t) {
			if (what)
#ifdef ELBOX_USE_IPV6
				log_d("%s timeout to %s[%s]", what, c->peer.ap_address, c->peer.ap_port);
#else			
				log_d("%s timeout to %s[%hu]", what, inet_ntoa(c->peer.sin_addr), ntohs(c->peer.sin_port));
#endif
			close_connection(c);
		}
		c = n;
	}
}

static void cleanup_connections(void)
{
	timeout_connections(waiting_connections.head, tuning.wait_timeout, debug ? "wait" : 0);
	timeout_connections(reading_connections.head, tuning.timeout, "read");
	timeout_connections(writing_connections.head, tuning.timeout, "write");
	timeout_connections(forked_connections.head, tuning.script_timeout, "script");
}

static void reap_children(void)
{
	pid_t pid;
	int errno_save, status, exitstatus, termsig;

	errno_save = errno;
	while (1) {
		pid = waitpid(-1, &status, WNOHANG);
		if (pid <= 0)
			break;
		if (WIFEXITED(status)) {
			++stats.exited_children;
			exitstatus = WEXITSTATUS(status);
			if (exitstatus || debug)
				log_d("child process %d exited with status %d", (int) pid, exitstatus);
		} else if (WIFSIGNALED(status)) {
			++stats.exited_children;
			termsig = WTERMSIG(status);
			log_d("child process %d killed by signal %d", (int) pid, termsig);
		} else
			log_d("child process %d stopped!?", (int) pid);
	}
	if (pid < 0 && errno != ECHILD)
		lerror("waitpid");
	errno = errno_save;
}

#ifndef INFTIM
#define INFTIM -1
#endif

static void dump_pollfds(int n, int r)
{
	char *buf, *b;
	int i;

	buf = malloc(5 * n + 10);
	if (buf == 0)
		return;
	if (r == 0) {
		b = buf + sprintf(buf, "fds:     ");
		for (i = 0; i < n; i++)
			b += sprintf(b, " %4d", pollfds[i].fd);
		log_d(buf);
		b = buf + sprintf(buf, "events:  ");
		for (i = 0; i < n; i++)
			b += sprintf(b, " %4hd", pollfds[i].events);
		log_d(buf);
	} else {
		b = buf + sprintf(buf, "revents: ");
		for (i = 0; i < n; i++)
			b += sprintf(b, " %4hd", pollfds[i].revents);
		log_d(buf);
	}
	free(buf);
}

/*------------------------------*/
/* Added/Modified by Paul Liu 20061220 */
/* Modified time check from time(0) to uptime */
/* Sync time function. Do not get time everytime, only sync it if it's necessary...*/
void sync_time(void)
{
	struct sysinfo info;
	sysinfo(&info);
	current_uptime = info.uptime;
	current_time = time(0);
}
/*------------------------------*/

void httpd_main(void)
{
	int rv, n, t, accepting;
	/*------------------------------*/
	/* Added/Modified by Paul Liu 20060324 */
	/* Add support for scheduled shutdown */
	long uphours, last_uptime, sig_uptime=0;
	/* Added/Modified by Paul Liu 20061212 */
	/* Add support for unix sock command server */
	if(NULL==(cmd_server.usock=usock_open(1, CMD_SOCK)))
		log_d("warning: fail to create unix socket at %s", CMD_SOCK);
    /* Added by Kwest Wan 20070928*/
	/* Fix bug: Add to save errno before sync time */
	int saved_errno;
#if CONFIG_RGBIN_BRCTL_WEB_REDIRECT
	char wr_enable[2];
#endif
	accepting = 1;

	/* Added/Modified by Paul Liu 20061220 */
	/* Modified time check from time(0) to uptime */
	sync_time();
	last_uptime = current_uptime;
	startuptime = current_time;
	/*------------------------------*/

	uphours = current_uptime / 3600;
	log_d("*** %s starting", server_version);
	log_d("NumConnections=%lu nfiles=%d", tuning.num_connections, nfiles);
	if (2 * tuning.num_connections + 4 > nfiles)
		log_d("warning: NumConnections is too high");
	while (gotsigterm == 0)
	{
		if (gotsighup)
		{
			gotsighup = 0;
			init_logs(0);
			if (debug) log_d("logs reopened");
		}
		if (gotsigusr1)
		{
			gotsigusr1 = 0;
			close_connections();
			log_d("connections closed");
		}
		if (gotsigusr2)
		{
			/*------------------------------*/
			/* Added/Modified by Paul Liu 20060324 */
			/* Add support for scheduled shutdown */
			if(!sig_uptime)
			{
				sig_uptime=current_uptime;
				log_d("servers will be closed in 3 secs");
			}
			else if(current_uptime-sig_uptime>3)
			{
				close_servers();
				gotsigterm=1;
				log_d("servers closed");
			}
			/*------------------------------*/
		}
		if (gotsigchld)
		{
			gotsigchld = 0;
			reap_children();
		}
		if (gotsigquit)
		{
			gotsigquit = 0;
			debug = debug == 0;
			if (debug) log_d("debugging turned on");
			else log_d("debugging turned off");
		}

		/*------------------------------*/
		/* Added by Paul Liu 20040326 */
		/* Add condition compile for DUMP */
#ifdef DUMP
		if (gotsigwinch)
		{
			gotsigwinch = 0;
			log_d("performing internal dump");
			internal_dump();
		}
#endif
		/*------------------------------*/
		n = 0;
		if (accepting && find_connection()) n = setup_server_pollfds(n);
		n = setup_connection_pollfds(n);
		n = setup_child_pollfds(n, forked_connections.head);
		if (n == 0 && accepting && stats.nconnections == 0)
		{
			log_d("no more sockets to poll from");
			break;
		}

		/*------------------------------*/
		/* Added by Paul Liu 20040603 */
		/* Add scheduling image burning */
		if(upload_file.flag>0) check_upgrad();
#if 0
		t = accepting ? (stats.nconnections ? 60000 : INFTIM) : 1000;
#else
		t = 1000;
#endif
		if (debug) dump_pollfds(n, 0);
		rv = poll(pollfds, n, t);

		/* Added by Kwest Wan 20070928*/
		/* Fix bug: save errno before sync time */
		saved_errno = errno;
		
		/* Added/Modified by Paul Liu 20061220 */
		/* Modified time check from time(0) to uptime */
		sync_time();
		/*------------------------------*/

		if (rv == -1)
		{
			if (saved_errno != EINTR) /* Fix bug: Modify variable errno to saved_errno, by Kwest Wan 20070928*/
			{
				lerror("poll");
				break;
			}
			else
			{
				if (debug) log_d("poll interrupted");
				continue;
			}
		}
		if (debug) dump_pollfds(n, 1);
		if (current_uptime != last_uptime)
		{
			if (accepting == 0) accepting = 1;
			if (current_uptime / 3600 != uphours)
			{
				uphours = current_uptime / 3600;
				init_logs(0);
				if (debug) log_d("logs rotated");
			}
		}
		if (rv)
		{
#if CONFIG_RGBIN_BRCTL_WEB_REDIRECT
			RGDBGET(wr_enable,2, "/wlan/inf:1/webredirect/enable");
			//RGDBGET(wr_dstip_flag,2, "/runtime/webredirect/daddr_ip_flag");
			//webprintf("1111 wr_enable=%s, wr_dstip_flag=%s\n",wr_enable, wr_dstip_flag);
			if(strncmp(wr_enable,"1",4) == 0) // webredirect enable
			{
					//system("brctl getstaaddr br0");
					system("brctl getdaddrip br0");	
			}
			//webprintf("2222 wr_enable=%s, wr_dstip_flag=%s\n",wr_enable, wr_dstip_flag);

#endif		
			/*------------------------------*/
			/* Added by Paul Liu 20061212 */
			/* Add support for unix sock command server */
			run_command();
			/*------------------------------*/

			if (accepting && run_servers() == -1) accepting = 0;
			run_connections();
		}
		if (current_uptime != last_uptime)
		{
			cleanup_connections();
			last_uptime = current_uptime;
		}
	}
	/*------------------------------*/
	/* Added by Paul Liu 20061212 */
	/* Close unix sock command server */
	if(cmd_server.usock)
	{
		usock_close(cmd_server.usock);
		memset(&cmd_server, 0, sizeof(cmd_server));
	}
	/*------------------------------*/

	log_d("*** shutting down");
}

int init_pollfds(size_t n)
{
	if (n == 0)
		return 0;
	pollfds = malloc(n * sizeof *pollfds);
	if (pollfds == 0) {
		log_d("init_pollfds: out of memory");
		return -1;
	}
	return 0;
}

static int new_pool(struct pool *p, size_t s)
{
	char *t;

	t = malloc(s);
	if (t == 0) {
		log_d("new_pool: out of memory");
		return -1;
	}
	p->floor = p->start = p->middle = p->end = t;
	p->ceiling = t + s;
	return 0;
}

int init_connections(size_t n)
{
	size_t i;
	struct connection *cn;

	connection_array = malloc(n * sizeof *connection_array);
	if (connection_array == 0) {
		log_d("init_connections: out of memory");
		return -1;
	}
	for (i = 0; i < n; i++) {
		cn = connection_array + i;
		if ((cn->r = malloc(sizeof *cn->r)) == 0) {
			log_d("init_connections: out of memory");
			return -1;
		}
		if (tuning.num_headers == 0)
			cn->r->headers = 0;
		else if ((cn->r->headers = malloc(tuning.num_headers * sizeof *cn->r->headers)) == 0) {
			log_d("init_connections: out of memory");
			return -1;
		}
		if (new_pool(&cn->header_input, tuning.input_buf_size) == -1)
			return -1;
		if (new_pool(&cn->output, tuning.buf_size) == -1)
			return -1;
		if (new_pool(&cn->client_input, tuning.script_buf_size) == -1)
			return -1;
		if (new_pool(&cn->script_input, tuning.script_buf_size) == -1)
			return -1;
		cn->r->cn = cn;
		cn->connection_state = HC_UNATTACHED;
		set_connection_state(cn, HC_FREE);
	}
	return 0;
}
