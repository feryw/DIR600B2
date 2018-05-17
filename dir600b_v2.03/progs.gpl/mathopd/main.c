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

/* Once Around */

static const char rcsid[] = "$Id: main.c,v 1.1.1.1 2005/05/19 10:53:06 r01122 Exp $";

#include <sys/types.h>
#include <sys/socket.h>
#ifdef INTERFACE_BIND_SUPPORT
#include <net/if.h>	/* for struct ifreq. */
#endif
#ifdef SOCK_DGRAM_SUPPORT
#include <netinet/in.h>
#include <arpa/inet.h>
#endif 
#include <sys/ioctl.h> 
#include <net/if.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <grp.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <time.h>
#include "mathopd.h"

#include <errno.h>
const char server_version[] = "Mathopd/1.5p6";

volatile sig_atomic_t gotsigterm;
volatile sig_atomic_t gotsighup;
volatile sig_atomic_t gotsigusr1;
volatile sig_atomic_t gotsigusr2;
volatile sig_atomic_t gotsigchld;
volatile sig_atomic_t gotsigquit;


#ifdef ELBOX_USE_IPV6
volatile sig_atomic_t gotsigwinch;
#else
/*------------------------------*/
/* Added by Paul Liu 20040326 */
/* Add condition compile for IMAP/REDIRECT/DUMP */
#ifdef DUMP
volatile sig_atomic_t gotsigwinch;
#endif
#endif

/* Added by Paul Liu 20040531 */
/* Add signature for upgrad fw */
char *g_signature=NULL;
/*------------------------------*/
int debug;
uint64_t fcm; /* should be mode_t */
int stayroot;
int amroot;
volatile int my_pid;
int nfiles;

static int am_daemon;
static char *progname;

static const char devnull[] = "/dev/null";

static int mysignal(int sig, void(*f)(int))
{
	struct sigaction act;

	act.sa_handler = f;
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;
	return sigaction(sig, &act, 0);
}

static void die(const char *t, const char *fmt, ...)
{
	va_list ap;

	if (fmt) {
		fprintf(stderr, "%s: ", progname);
		va_start(ap, fmt);
		vfprintf(stderr, fmt, ap);
		fprintf(stderr, "\n");
		va_end(ap);
	}
	if (t)
		perror(t);
#ifdef ELBOX_USE_IPV6		
	if (pid_filename) unlink(pid_filename);
#endif	
	exit(1);
}

static void startup_server(struct server *s)
{
	int onoff;
#ifdef ELBOX_USE_IPV6
	struct server_sockopts *o;
	s->fd = socket(s->family, s->socktype, s->protocol);
#else
	struct sockaddr_in sa;

	/*------------------------------*/
	/* Modified/Added by Paul Liu 20061003 */
	/* Add support for UDP type server */
	if(s->udp)
		s->fd = socket(AF_INET, SOCK_DGRAM, 0);
	else
		s->fd = socket(AF_INET, SOCK_STREAM, 0);
	/*------------------------------*/

#endif	

	if (s->fd == -1)
		die("socket", 0);
	onoff = 1;
	if (setsockopt(s->fd, SOL_SOCKET, SO_REUSEADDR, (char *) &onoff, sizeof onoff) == -1)
		die("setsockopt", "cannot set re-use flag");
#ifdef ELBOX_USE_IPV6
	o = s->options;
	while (o) {
		if (setsockopt(s->fd, o->ss_level, o->ss_optname, o->ss_optval, o->ss_optlen) == -1)
			die("setsockopt", 0);
		o = o->next;
	}
#endif
	fcntl(s->fd, F_SETFD, FD_CLOEXEC);
	fcntl(s->fd, F_SETFL, O_NONBLOCK);


#ifdef ELBOX_USE_IPV6
#ifdef INTERFACE_BIND_SUPPORT
	if (s->interface)
	{
		struct ifreq interface;
		strncpy(interface.ifr_ifrn.ifrn_name, s->interface, IFNAMSIZ);
		if (setsockopt(s->fd, SOL_SOCKET, SO_BINDTODEVICE, (char *)&interface, sizeof(interface)) < 0)
			die("setsockopt", "cannot bind to device %s", s->interface);
	}
#endif

	if (bind(s->fd, s->server_addr, s->server_addrlen) == -1)
		die("bind", "cannot start up server at %s port %s", s->addr ? s->addr : "[any]", s->port);
#ifdef SOCK_DGRAM_SUPPORT
	if (s->socktype == SOCK_DGRAM && s->addr)
	{
		struct in_addr inaddr;
		struct ip_mreq mreq;
		struct ifreq ifr;

		inet_aton(s->addr, &inaddr);
		if (IN_MULTICAST(htonl(inaddr.s_addr)) && s->interface)
		{
			/* get the IP address of this interface. */
			memset(&ifr, 0, sizeof(struct ifreq));
			strcpy(ifr.ifr_name, s->interface);
			if (ioctl(s->fd, SIOCGIFADDR, &ifr)==0)
			{
				memset(&mreq, 0, sizeof(mreq));
				mreq.imr_multiaddr.s_addr = inaddr.s_addr;
				mreq.imr_interface.s_addr = ((struct sockaddr_in *)&(ifr.ifr_addr))->sin_addr.s_addr;
				if (setsockopt(s->fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0)
					die("setsockopt", "cannot add multicast membership - %s", s->addr);
			}
			else
			{
				printf("httpd: unable to get ipaddr from '%s'.\n", s->interface);
			}
		}
	}
	else
#endif
	if (listen(s->fd, s->backlog) == -1)
		die("listen", 0);
#else
	memset(&sa, 0, sizeof sa);
	sa.sin_family = AF_INET;
	if (IN_MULTICAST(htonl(s->addr.s_addr))) sa.sin_addr.s_addr = INADDR_ANY;
	else sa.sin_addr = s->addr;
	sa.sin_port = htons(s->port);
	/*------------------------------*/
	/* Added by Paul Liu 20060324 */
	/* Add support for interface name bind */
	if(debug)
		fprintf(stderr, "start server at %s(%s):%d%s", sa.sin_addr.s_addr?inet_ntoa(sa.sin_addr):"ADDRESS_ANY", s->interface?s->interface:"IF_ANY", sa.sin_port, s->udp==1?" UDP\n":"\n");

	if(s->interface)
	{
		struct ifreq interface;
		strncpy(interface.ifr_ifrn.ifrn_name, s->interface, IFNAMSIZ);
		if(setsockopt(s->fd, SOL_SOCKET, SO_BINDTODEVICE,(char *)&interface, sizeof(interface))<0)
			die("bind", "cannot start up server at %s port %lu", s->interface, s->port);
	}
	if (bind(s->fd, (struct sockaddr *) &sa, sizeof sa) == -1)
		die("bind", "cannot start up server at %s port %lu", inet_ntoa(sa.sin_addr), s->port);

	/*------------------------------*/
	/* Modified/Added by Paul Liu 20061003 */
	/* Add support for UDP type server */
	if (s->udp && s->addr.s_addr)
	{
		struct ip_mreq mreq;
		mreq.imr_multiaddr.s_addr = s->addr.s_addr;
		mreq.imr_interface.s_addr = htonl(INADDR_ANY);
		if (setsockopt(s->fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq))<0)
			die("setsockopt", "cannot add multicast membership");
	}
	else if (listen(s->fd, s->backlog) == -1)
		die("listen", 0);
	/*------------------------------*/
#endif
}

static void sighandler(int sig)
{
/*
 * If we use vfork(), we must ignore all signals that are delivered when we
 * are running as a child process (in particular during the window of time
 * between setuid(u) and execve()). Checking getpid() against my_pid in the
 * signal handler appears to be the quick-and-dirtiest way to do this.
 */
#ifdef HAVE_VFORK
	if (getpid() != my_pid)
		return;
#endif
	switch (sig) {
	case SIGTERM:
	case SIGINT:
		gotsigterm = 1;
		break;
	case SIGHUP:
		gotsighup = 1;
		break;
	case SIGUSR1:
		gotsigusr1 = 1;
		break;
	case SIGUSR2:
		gotsigusr2 = 1;
		break;
	case SIGCHLD:
		gotsigchld = 1;
		break;
	case SIGQUIT:
		gotsigquit = 1;
		break;
	/*------------------------------*/
	/* Added by Paul Liu 20040326 */
	/* Add condition compile for IMAP/REDIRECT/DUMP */
#ifdef DUMP
	case SIGWINCH:
		gotsigwinch = 1;
		break;
#endif
	/*------------------------------*/
	}
}

int main(int argc, char *argv[])
{
	int c, i, version, pid_fd, null_fd, tee;
	struct server *s;
	char buf[10];
	struct rlimit rl;
	const char *message;
	const char *config_filename;

	my_pid = getpid();
	progname = argv[0];
#ifdef ELBOX_USE_IPV6
	am_daemon = 1;
#else
	am_daemon = 0;
#endif	
	version = 0;
	config_filename = 0;
	tee = 0;
	while ((c = getopt(argc, argv, "nds:vf:t")) != EOF) {
		switch(c) {
		case 'n':
			am_daemon = 0;
			break;
		case 'd':
			debug = 1;
			break;
		case 'v':
			version = 1;
			break;
		case 'f':
			if (config_filename == 0)
				config_filename = optarg;
			else
				die(0, "You may not specify more than one configuration file.");
			break;
		/*------------------------------*/
		/* Added by Paul Liu 20040531 */
		/* Add signature for upgrad fw */
		case 's':
			g_signature=optarg;
			break;
		/*------------------------------*/
		case 't':
			tee = 1;
			break;
		default:
			die(0, "usage: %s [ -ndsvt ] [ -f configuration_file ]", progname);
			break;
		}
	}
	if (version) {
		fprintf(stderr, "%s\n", server_version);
		return 0;
	}
	if (getrlimit(RLIMIT_NOFILE, &rl) == -1)
		die("getrlimit", 0);
	nfiles = rl.rlim_cur = rl.rlim_max;
	setrlimit(RLIMIT_NOFILE, &rl);
	if (am_daemon)
		for (i = 3; i < nfiles; i++)
			close(i);
	null_fd = open(devnull, O_RDWR);
	if (null_fd == -1)
		die("open", "Cannot open %s", devnull);
	while (null_fd < 3) {
		null_fd = dup(null_fd);
		if (null_fd == -1)
			die("dup", 0);
	}
	message = config(config_filename);
	if (message)
		die(0, "%s", message);
	s = servers;
	while (s) {
		startup_server(s);
		s = s->next;
	}
	if (rootdir) {
		if (chroot(rootdir) == -1)
			die("chroot", 0);
		if (chdir("/") == -1)
			die("chdir", 0);
	}
	/*------------------------------*/
	/* Added by Paul Liu 20040326 */
	/* Add condition compile for UID/GID */
#ifndef ELBOX_USE_IPV6
#ifdef USE_ID
	setuid(geteuid());
	if (geteuid() == 0) {
		if (server_uid == 0)
			die(0, "No user specified.");
		if (setgroups(0, 0) == -1)
			if (setgroups(1, &server_gid) == -1)
				die("setgroups", 0);
		if (setgid(server_gid) == -1)
			die("setgid", 0);
		if (stayroot) {
			if (seteuid(server_uid) == -1)
				die("seteuid", 0);
			amroot = 1;
		} else {
			if (setuid(server_uid) == -1)
				die("setuid", 0);
		}
	}
#endif
#endif
	/*------------------------------*/

	if (getrlimit(RLIMIT_CORE, &rl) == -1)
		die("getrlimit", 0);
	if (coredir) {
		rl.rlim_cur = rl.rlim_max;
		if (chdir(coredir) == -1)
			die("chdir", 0);
	} else {
		rl.rlim_cur = 0;
		chdir("/");
	}
	setrlimit(RLIMIT_CORE, &rl);
	umask(fcm);
	if (pid_filename) {
		pid_fd = open(pid_filename, O_WRONLY | O_CREAT, 0666);
		if (pid_fd == -1)
			die("open", "Cannot open PID file");
	} else
		pid_fd = -1;
#ifdef ELBOX_USE_IPV6
	current_time = time(0);
#else
	sync_time();
#endif		

	if (init_logs(tee) == -1)
		die("open", "Cannot open log files");
	dup2(null_fd, 0);
	dup2(null_fd, 1);
	dup2(null_fd, 2);
	close(null_fd);
#ifdef ELBOX_USE_IPV6
	if (am_daemon) {
		if (fork())
			_exit(0);
		setsid();
		if (fork())
			_exit(0);
		for (i = 1; i < tuning.num_processes; i++) {
			if (fork() == 0) {
				if (pid_fd != -1) {
					close(pid_fd);
					pid_fd = -1;
				}
				break;
			}
		}
	}
#else
	/* Added by Paul Liu 20040428 */
	/* Do not use fork under uclinux */
#ifndef __uClinux__
	if (am_daemon) {
		if (fork())
			_exit(0);
		setsid();
		if (fork())
			_exit(0);
	}
#endif
	/*------------------------------*/
#endif
	mysignal(SIGCHLD, sighandler);
	mysignal(SIGHUP, sighandler);
	mysignal(SIGTERM, sighandler);
	mysignal(SIGINT, sighandler);
	mysignal(SIGQUIT, sighandler);
	mysignal(SIGUSR1, sighandler);
	mysignal(SIGUSR2, sighandler);
	mysignal(SIGPIPE, SIG_IGN);
	mysignal(SIGWINCH, sighandler);
	my_pid = getpid();
	if (pid_fd != -1) {
		ftruncate(pid_fd, 0);
#ifdef ELBOX_USE_IPV6
		if (tuning.num_processes > 1)
			sprintf(buf, "-%d\n", (int) getpgrp());
		else
#endif
		sprintf(buf, "%d\n", my_pid);
		write(pid_fd, buf, strlen(buf));
		close(pid_fd);
	}
	if (init_buffers() == -1)
		return 1;
	httpd_main();
#ifdef ELBOX_USE_IPV6	
	if (pid_filename) unlink(pid_filename);
#endif	
	return 0;
}

/*------------------------------*/
/* Added by Paul Liu 20040326 */
/* Add condition compile for UID/GID */
#if defined(USE_ID) ///|| defined(ELBOX_USE_IPV6)
pid_t spawn(const char *program, char *const argv[], char *const envp[], int fd, int efd, uid_t u, gid_t g, const char *curdir)
#else
pid_t spawn(const char *program, char *const argv[], char *const envp[], int fd, int efd, const char *curdir)
#endif
/*------------------------------*/
{
	pid_t	pid;
	struct	rlimit rl;

#ifdef ELBOX_USE_IPV6
#ifdef HAVE_VFORK
	pid = vfork();
#else
	pid = fork();
#endif
#else
	/*------------------------------*/
	/* Modified by Paul Liu 20060116 */
	/* Do not use vfork, because we don't want NOMMU boards anymore */
#if 0 //HAVE_VFORK
	pid = vfork();
#else
	pid = fork();
#endif
	/*------------------------------*/
#endif
	switch (pid){
	default:
		++stats.forked_children;
		if (debug) 
                        log_d("child process %d created", (int) pid);
#ifdef ELBOX_USE_IPV6
	break;
#else
		return pid;
#endif
	case -1:
		lerror("spawn: failed to create child process");
#ifdef ELBOX_USE_IPV6
	break;
#else
		return -1;
#endif
	case 0:
		setpgid(0, 0);
		mysignal(SIGPIPE, SIG_DFL);
		if (coredir){
			rl.rlim_cur = rl.rlim_max = 0;
			setrlimit(RLIMIT_CORE, &rl);
		}
		dup2(fd, 0);
		dup2(fd, 1);
		if (efd != -1) 
                        dup2(efd, 2);

#ifdef ELBOX_USE_IPV6
		///if (chdir(curdir) == -1)
		///	_exit(5);
		execve(program, argv, envp);
		_exit(6);
	}
	return pid;
#else

		/*------------------------------*/
		/* Added by Paul Liu 20040326 */
		/* Add condition compile for UID/GID */
#if USE_ID
		if (u)
		{
			if (setuid(0) == -1) _exit(1);
			if (setgid(g) == -1) _exit(2);
			if (setuid(u) == -1) _exit(3);
		}
		if (getuid() == 0 || geteuid() == 0) _exit(4);
#endif

		/* Modified by Paul Liu 20040326 */
		/* Do not exit when curdir==NULL(None exists file) */
#if 0
		if (chdir(curdir) == -1) _exit(5);
#endif

		chdir(curdir);

		/* Modified by Paul Liu 20040326 */
		/* Change "execve"(which cause crash in WP3210) to "system" */
		/* Change "system" to "execve" now, cause it will crash other paltform */
#ifdef HOME_PORTAL	/*Teresa, for HomePortal 2007/10/18 02:37�U��*/
		int	i=0;
		char exestr[255];
		for(i=0;envp[i];i++) putenv(envp[i]);
		strcpy(exestr,program);
		for(i=1;argv[i];i++) sprintf(exestr, "%s '%s'",exestr, argv[i]);
		system(exestr);
#else
#if 1
		execve(program, argv, envp);
#else
		char exestr[255];
		for(i=0;envp[i];i++) putenv(envp[i]);
		strcpy(exestr,program);
		for(i=1;argv[i];i++) sprintf(exestr, "%s '%s'",exestr, argv[i]);
		system(exestr);
#endif
#endif
		/*------------------------------*/
		_exit(6);
	}
	return -1; /* not reached */
#endif
}
