/*
 *   Copyright 1996, 1997, 1998, 1999, 2000, 2001, 2002, 2003 Michiel Boland.
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

/* In der Halle des Bergk"onigs */

#ifndef _mathopd_h
#define _mathopd_h

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdint.h> /* (u)intmax_t */
#if __WORDSIZE == 32
#undef uint64_t
#define uint64_t unsigned long int
#undef int64_t
#define int64_t long int
#endif
#include "elbox_config.h"

#ifndef __GNUC__
#define __attribute__(x)
#endif

#define CGI_MAGIC_TYPE "CGI"
#define IMAP_MAGIC_TYPE "Imagemap"
#define REDIRECT_MAGIC_TYPE "Redirect"
#define DUMP_MAGIC_TYPE "Dump"
/*------------------------------*/
/* Added by Paul Liu 20040422 */
/* Add internal function call */
#define INTERNAL_MAGIC_TYPE "Internal"

/* Added by Paul Liu 20050218 */
/* Change user name length to 80 chars */
#define DEFAULT_NAME_SIZE 81
/*------------------------------*/

/*------------------------------*/
/* Added by Paul Liu 20050913 */
/* Add support for upload size control. */
#define DEFAULT_UPLOAD_SIZE 4096000
/*------------------------------*/
#define DEFAULT_BUF_SIZE 12288
#define DEFAULT_INPUT_BUF_SIZE 2048
#define DEFAULT_NUM_CONNECTIONS 64
#define DEFAULT_TIMEOUT 60
#define DEFAULT_UMASK 022
#define DEFAULT_NUM_HEADERS 100
#define DEFAULT_SCRIPT_TIMEOUT 60
#define DEFAULT_SCRIPT_BUF_SIZE 4096
#define DEFAULT_BACKLOG 128
#define DEFAULT_WAIT_TIMEOUT 60

#define STRLEN 400
#define PATHLEN (2 * STRLEN)

#ifdef ELBOX_USE_IPV6
#define CONF_SMALLINT_MAX 64
#endif

enum {
	ALLOW,
	DENY,
	APPLY,
	NOAPPLY
};

enum {
	CLASS_FILE = 1,
	CLASS_SPECIAL,
	CLASS_EXTERNAL
};

enum {
	M_UNKNOWN,
	M_HEAD,
	M_GET,
	M_POST
};

enum connection_state {
	HC_UNATTACHED,
	HC_FREE,
	HC_FORKED,
	HC_READING,
	HC_WRITING,
	HC_WAITING
};

enum {
	ML_CTIME,
	ML_USERNAME,
	ML_REMOTE_ADDRESS,
	ML_REMOTE_PORT,
	ML_LOCAL_ADDRESS,
	ML_LOCAL_PORT,
	ML_SERVERNAME,
	ML_METHOD,
	ML_URI,
	ML_VERSION,
	ML_STATUS,
	ML_CONTENT_LENGTH,
	ML_REFERER,
	ML_USER_AGENT,
	ML_BYTES_READ,
	ML_BYTES_WRITTEN,
	ML_QUERY_STRING,
	ML_TIME_TAKEN,
	ML_MICRO_TIME
};

enum {
	SI_DONOTCHANGE,
	SI_CHANGETOFIXED,
	SI_CHANGETOOWNER
};

enum {
	TYPE_CA,
	TYPE_CERTIFICATE,
#ifndef ELBOX_PROGS_GPL_MATHOPD_EXTERNAL_FILE
	TYPE_PRIVATE_KEY
#else
	TYPE_PRIVATE_KEY,
	TYPE_STUNNEL_CERT,
	TYPE_STUNNEL_KEY
#endif
};

struct pool {
	char *floor;
	char *ceiling;
	char *start;
	char *middle;
	char *end;
	char state;
};

struct access {
	int type;
	uint64_t mask;
	uint64_t addr;
	struct access *next;
};

struct mime {
	int class;
	char *ext;
	char *name;
	struct mime *next;
};

struct simple_list {
	char *name;
	struct simple_list *next;
};

struct control {
	char *alias;
/*------------------------------*/
/* Added by Paul Liu 20060317 */
/* Add support for Alias file which allow multi-alias in a control block . */
	char *alias_file;
/*------------------------------*/
	int path_args_ok;
	int exact_match;
/*------------------------------*/
/* Added by Paul Liu 20050802 */
/* Add support for session control. */
    int session_control;
    uint64_t session_idletime;

/* Added by Paul Liu 20050902 */
/* Add maximum session number for session control. */
    uint64_t session_max;

/* Added by Paul Liu 20050913 */
/* Add file filter for session control. */
	struct simple_list *session_filter;
/*------------------------------*/
	struct simple_list *index_names;
	struct access *accesses;
	struct mime *mimes;
	struct control *next;
	struct simple_list *locations;
	struct access *clients;
	char *admin;
	char *realm;
	char *userfile;
	char *error_401_file;
	char *error_403_file;
	char *error_404_file;
/*------------------------------*/
/* Added by Paul Liu 20050807 */
/* Add support for Info Pages. */
	char *error_fwup_file;
	char *error_cfgup_file;
	char *info_fwrestart_file;
	char *info_cfgrestart_file;
/*------------------------------*/
	int do_crypt;
	char *child_filename;
	struct simple_list *exports;
	int script_identity;
/*------------------------------*/
/* Added by Paul Liu 20040326 */
/* Add condition compile for UID/GID */
#if defined(USE_ID) ///|| defined(ELBOX_USE_IPV6)
	uid_t script_uid;
	gid_t script_gid;
#endif
/*------------------------------*/
	int allow_dotfiles;
	int user_directory;
	struct simple_list *putenvs;
	struct simple_list *extra_headers;
	int path_info_ok;
	char *auto_index_command;
	int sanitize_path;
};

struct virtual {
	char *host;
	struct virtual *next;
	struct vserver *vserver;
	int anyhost;
};

struct vserver {
	struct control *controls;
	struct vserver *next;
};

#ifdef ELBOX_USE_IPV6
struct server_sockopts {
	int ss_level;
	int ss_optname;
	void *ss_optval;
	socklen_t ss_optlen;
	struct server_sockopts *next;
};
#endif
struct server {
	int fd;
/*------------------------------*/
/* Added by Paul Liu 20060324 */
/* Add support for interface name bind */
	char *interface;

/* Added by Paul Liu 20061003 */
/* Add support for UDP type server */
	int	udp;

/* Added by Paul Liu 20061016  */
/* Add support for user defined Server Name */
	char *name;
/*------------------------------*/
#ifndef ELBOX_USE_IPV6
	uint64_t port;
	struct in_addr addr;
#endif	
	struct virtual *children;
	struct control *controls;
	struct server *next;
	int pollno;
	struct vserver *vservers;
	uint64_t backlog;
#ifdef ELBOX_USE_IPV6
	char *addr;
	char *port;
	int family;
	int socktype;
	int protocol;	
	struct server_sockopts *options;
	struct sockaddr *server_addr;	
	size_t server_addrlen;	
#ifdef INTERFACE_BIND_SUPPORT
	char * interface;
#endif
#ifdef SERVER_NAME_SUPPORT
	char * servername;
#endif
#ifdef SERVER_ID_SUPPORT
	char * serverid;
#endif
#endif	
};

struct request_header {
	const char *rh_name;
	const char *rh_value;
};

struct request {
	struct connection *cn;
	struct virtual *vs;
	char *user_agent;
	char *referer;
	char *authorization;
	char *host;
	char *in_content_type;
	char *in_content_length;
/*------------------------------*/
/* Added by Paul Liu 20040426 */
/* Check for multipart upload */
	char *multi_boundary;
/*------------------------------*/
	char *ims_s;
	char path[PATHLEN];
	char path_translated[PATHLEN];
	char path_args[PATHLEN];
	const char *content_type;
	int num_content;
	int class;
	intmax_t content_length;
	time_t last_modified;
	time_t ims;
	char *location;
	char *method_s;
	char *url;
	char *args;
	char *version;
	int protocol_major;
	int protocol_minor;
	int method;
	int status;
	struct control *c;
	struct stat finfo;
	int isindex;
	const char *error_file;
/*------------------------------*/
/* Added by Paul Liu 20040218 */
/* Change user name length to 80 chars */
	char user[DEFAULT_NAME_SIZE];

/* Added by Paul Liu 20050614 */
/* Add support for user password Env. */
    char passwd[DEFAULT_NAME_SIZE];

/* Added by Paul Liu 20050802 */
/* Add support for session control. */
    int sessionid;
/*------------------------------*/

	char newloc[PATHLEN];
	size_t location_length;
	size_t nheaders;
	struct request_header *headers;
	char *range_s;
	char *if_range_s;
	time_t if_range;
	int range;
	uintmax_t range_floor;
	uintmax_t range_ceiling;
	uintmax_t range_suffix;
	uintmax_t range_total;
	char *ius_s;
	time_t ius;
	char rhost[STRLEN];
	char *in_transfer_encoding;
	size_t in_mblen;
	char curdir[PATHLEN];
	int send_continue;
	int forked;
};

struct cgi_header {
	const char *name;
	size_t namelen;
	const char *value;
	size_t len;
};

struct pipe_params {
	int state;
	size_t imax;
	int chunkit;
	int nocontent;
	int haslen;
	size_t pmax;
};

#ifdef ELBOX_USE_IPV6
struct addrport {
	char ap_address[58];
	char ap_port[6];
};
#endif
struct connection {
	struct connection *next;
	struct connection *prev;
	enum connection_state connection_state;
	struct request *r;
	struct server *s;
	int fd;
	int rfd;
#ifdef ELBOX_USE_IPV6	
	struct addrport peer;
	struct addrport sock;	
#else	
	struct sockaddr_in peer;
	struct sockaddr_in sock;
#endif	
/*------------------------------*/
/* Added/Modified by Paul Liu 20061220 */
/* Modified time check from time(0) to uptime */
	long t;
/*------------------------------*/
	struct pool header_input;
	struct pool output;
	struct pool client_input;
	struct pool script_input;
	int keepalive;
	int pollno;
	int rpollno;
	uintmax_t nread;
	uintmax_t nwritten;
	intmax_t left;
	int logged;
	struct timeval itv;
	struct pipe_params pipe_params;
	off_t file_offset;
	int havefile;
#ifdef ELBOX_USE_IPV6		
//#ifdef SOCK_DGRAM_SUPPORT
	struct sockaddr_storage sa_remote;
	struct sockaddr_storage sa_local;
//#endif
#endif

};

struct connection_list {
	struct connection *head;
	struct connection *tail;
};

struct tuning {
/*------------------------------*/
/* Added by Paul Liu 20050913 */
/* Add support for upload size control. */
	uint64_t max_uploadsize;
/*------------------------------*/
	uint64_t buf_size;
	uint64_t input_buf_size;
	uint64_t num_connections;
	uint64_t timeout;
	int accept_multi;
	uint64_t num_headers;
	uint64_t script_timeout;
	uint64_t script_buf_size;
	int clobber;
	uint64_t wait_timeout;
#ifdef ELBOX_USE_IPV6	
	int num_processes; 
	int greedy;	
#endif
};

struct statistics {
	int nconnections;
	int maxconnections;
	uint64_t nrequests;
	uint64_t forked_children;
	uint64_t exited_children;
	uint64_t accepted_connections;
	uint64_t pipelined_requests;
};

/*------------------------------*/
/* Modified by Paul Liu 20060324 */
/* Add a struct for multipart upload */
typedef struct _upload{
char	*file_ptr;
char	*file_name;
char	*data;
int		file_length;
int		length;
int		flag;
long	uptime;
}upload;
/*------------------------------*/


/* main */

extern const char server_version[];
extern volatile sig_atomic_t gotsigterm;
extern volatile sig_atomic_t gotsighup;
extern volatile sig_atomic_t gotsigusr1;
extern volatile sig_atomic_t gotsigusr2;
extern volatile sig_atomic_t gotsigchld;
extern volatile sig_atomic_t gotsigquit;
/*------------------------------*/
/* Added by Paul Liu 20040326 */
/* Add condition compile for IMAP/REDIRECT/DUMP */
#ifdef DUMP
extern volatile sig_atomic_t gotsigwinch;
#endif
/*------------------------------*/
extern int debug;
extern uint64_t fcm;
extern int stayroot;
extern int amroot;
extern volatile int my_pid;
extern int nfiles;
/*------------------------------*/
/* Added by Paul Liu 20040326 */
/* Add condition compile for UID/GID */
#if defined(USE_ID) ///|| defined(ELBOX_USE_IPV6)
extern pid_t spawn(const char *, char *const[], char *const[], int, int, uid_t, gid_t, const char *);
#else
extern pid_t spawn(const char *, char *const[], char *const[], int, int, const char *);
#endif
/*------------------------------*/

/* config */

extern struct tuning tuning;
extern char *pid_filename;
extern char *log_filename;
extern char *error_filename;
extern char *rootdir;
extern char *coredir;
extern struct server *servers;
extern uid_t server_uid;
extern gid_t server_gid;
extern int log_columns;
extern int *log_column;
extern int log_gmt;
extern const char *config(const char *);
extern int init_buffers(void);

/* core */

extern struct statistics stats;
extern time_t startuptime;
/*------------------------------*/
/* Added/Modified by Paul Liu 20061220 */
/* Modified time check from time(0) to uptime */
extern time_t current_time;
extern time_t current_uptime;
/*------------------------------*/
extern struct pollfd *pollfds;
extern struct connection *connection_array;
extern void set_connection_state(struct connection *, enum connection_state);
extern int reinit_connection(struct connection *);
extern void close_connection(struct connection *);
/*------------------------------*/
/* Added/Modified by Paul Liu 20061220 */
/* Modified time check from time(0) to uptime */
extern void sync_time(void);
/*------------------------------*/
extern void httpd_main(void);
extern int init_pollfds(size_t);
extern int init_connections(size_t);

/* request */

extern char *rfctime(time_t, char *);
extern struct control *faketoreal(char *, char *, struct request *, int, int);
extern int pool_print(struct pool *, const char *, ...)
	__attribute__((format(printf, 2, 3)));
extern void init_request(struct request *);
extern int process_request(struct request *);
extern int cgi_error(struct request *);

/*------------------------------*/
/* Added by Paul Liu 20040422 */
/* Add internal function call */
extern int check_session(struct request *);
/*------------------------------*/

/* imap */

extern int process_imap(struct request *);

/* cgi */

extern int process_cgi(struct request *);

/* dump */

extern int process_dump(struct request *);
extern void internal_dump(void);

/*------------------------------*/
/* Added by Paul Liu 20040422 */
/* Add internal function call */
extern int process_internal(struct request *);

/* base64 */

/* Modified by Paul Liu 20050614 */
/* Add support for user password Env. */
extern int webuserok(const char *, const char *, char *, int, char *, int, int);
extern int get_webuser(const char *, char *, int, char *, int, int);
/*------------------------------*/

/* redirect */

extern int process_redirect(struct request *);

/* util */

extern int unescape_url(const char *, char *);
extern int unescape_url_n(const char *, char *, size_t);
extern void sanitize_host(char *);

/* log */

extern int init_log_buffer(size_t);
extern void log_request(struct request *);
extern int open_log(const char *);
extern int init_logs(int);
extern void log_d(const char *, ...)
	__attribute__((format(printf, 1, 2)));
extern void lerror(const char *);
/* added by Leo, 2007/05/21 16:15:59 */
#undef WEB_DEBUG
#ifdef WEB_DEBUG
void _webprintf_(const char *file,const char *func, int line, const char * format, ...);
#define webprintf(args...) _webprintf_(__FILE__,__FUNCTION__,__LINE__,##args)
#else
#define webprintf(args...)
#endif
/* ********************************* */

/* stub */

extern int init_cgi_headers(void);
extern void pipe_run(struct connection *);
extern void init_child(struct connection *, int);
extern int setup_child_pollfds(int, struct connection *);

#if defined LINUX_SENDFILE || defined FREEBSD_SENDFILE
/* sendfile */

extern int set_nopush(int, int);
extern off_t sendfile_connection(struct connection *);
#endif

/* upload */
extern void rlt_page(struct pool *p, char *pFName);
extern int upload_image(struct request *r, struct pool *p);
extern int upload_config(struct request *r, struct pool *p);
extern void check_upgrad(void);

/* util */

#if defined(ELBOX_USE_IPV6) && ( \
	defined(ELBOX_MODEL_DAP2590) || \
	defined(ELBOX_MODEL_DAP2690))
extern void mdelay(const int n);
#endif

#endif /* end of _mathopd_h */

