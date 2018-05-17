/* vi: set sw=4 ts=4: */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lrgbin.h"

#define DEFAULT_TEMP	"/www/locale/en/log"

#define TEMP_SYS		"sysact.tmp"
#define TEMP_ATT		"attack.tmp"
#define TEMP_DRP		"drops.tmp"
#define TEMP_NTC		"notice.tmp"

/***************************************************************************/
static parray_t t_sysact;
static parray_t t_attack;
static parray_t t_drops;
static parray_t t_notice;

/***************************************************************************/
extern char *o_tmppath;		/* where the template files are. */
extern int	asyslogflag;
extern void dbprint(char *pstrFormat, ...);

/* clean up all allocated memory and exit. */
void cleanup_tlog()
{
	lrgbin_pa_destroy(&t_sysact,1);
	lrgbin_pa_destroy(&t_attack,1);
	lrgbin_pa_destroy(&t_drops,1);
	lrgbin_pa_destroy(&t_notice,1);
}

/* read template file. */
static void read_template(parray_t * template, char * file)
{
	char fname[256];
	FILE * fh=NULL;
	int idx;
	char * line;
	char * iptr;
	char * mptr;

	if (o_tmppath)
	{
		sprintf(fname, "%s/%s", o_tmppath, file);
		fh = fopen(fname, "r");
	}
	if(!fh)
	{
		sprintf(fname, "%s/%s", DEFAULT_TEMP, file);
		fh = fopen(fname, "r");
	}

	if (fh)
	{
		while ((line = lrgbin_get_line_from_file(fh, 1))!=NULL)
		{
			iptr = strchr(line, ':');
			if (iptr)
			{
				mptr = strchr(iptr, '>');
				if (mptr)
				{
					iptr++;
					*mptr++ = '\0';
					idx = atoi(iptr);

					lrgbin_pa_set_nth(template, idx, strdup(mptr), 1);
				}
			}
			free(line);
		}
		fclose(fh);
	}
}

static int read_params(char * string, parray_t * params)
{
	int cnt=0;
	char * ptr=NULL;
	unsigned int state=0;

	if (!string) return 0;

	while (string && *string)
	{
		switch (state & 1)
		{
		case 0:
			string = strchr(string, '[');
			if (string)
			{
				string++;
				ptr = string;
				state++;
			}
			break;
		case 1:
			string = strchr(string, ']');
			if (string)
			{
				*string++ = '\0';
				lrgbin_pa_set_nth(params, cnt, strdup(ptr), 1);
				cnt++;
				state++;
			}
			break;
			
		}
	}

	return cnt;
}

void patch_line(char *msg, char *buf, int size_buf)
{
	char * message;
	int idx, cnt;
	parray_t * template=NULL;
	parray_t params;
	char * realmsg;

	if(!msg || !buf || size_buf<=7)	return;

	msg = strchr(msg, ':');
	if(!msg++)	return;
	message = lrgbin_eatwhite(msg);

	/* Fixed by Kwest, 2008.10.8  */
	if		(strncmp(message, "SYS:", 4)==0 && (asyslogflag & 0x01)) template = &t_sysact;
	else if	(strncmp(message, "ATT:", 4)==0 && (asyslogflag & 0x04)) template = &t_attack;
	else if	(strncmp(message, "DRP:", 4)==0 && (asyslogflag & 0x08)) template = &t_drops;
	else if	(strncmp(message, "NTC:", 4)==0 && (asyslogflag & 0x10)) template = &t_notice;
	else return;

	lrgbin_pa_init(&params, 0);
	memset(buf, 0, size_buf);

	if (template)
	{
		int	len=0;
		message += 4;
		for (idx = 0; *message >= '0' && *message <= '9'; message++) idx = idx*10 + *message - '0';
		cnt = read_params(message, &params);
		realmsg = lrgbin_pa_get_nth(template, idx);

		while (realmsg && *realmsg)
		{
			if (*realmsg == '$')
			{
				realmsg++;
				if (*realmsg >= '0' && *realmsg <= '9')
				{
					for (idx = 0; *realmsg >= '0' && *realmsg <= '9'; realmsg++) idx = idx*10 + *realmsg - '0';
					message = lrgbin_pa_get_nth(&params, idx);
					if (message)
					{
						if(len+strlen(message)>=size_buf)	goto exit_patch;
						snprintf(buf+len, size_buf-len, "%s", message);
						len+=strlen(message);
					}
				}
				else
				{
					if(len>=size_buf)	goto exit_patch;
					snprintf(buf+len, size_buf-len, "%c", *realmsg++);
					len++;
				}
			}
			else
			{
				if(len>=size_buf)	goto exit_patch;
				snprintf(buf+len, size_buf-len, "%c", *realmsg++);
				len++;
			}
		}
	}
	else
	{
		snprintf(buf, size_buf, "%s", message);
	}

exit_patch:
	lrgbin_pa_destroy(&params, 1);
//	printf("\"\n");
}

void init_tlog()
{
	lrgbin_pa_init(&t_sysact,	0);
	lrgbin_pa_init(&t_attack,	0);
	lrgbin_pa_init(&t_drops,	0);
	lrgbin_pa_init(&t_notice,	0);
	/* Fixed by Kwest, 2008.10.08 */
	if((asyslogflag & 0x01))	read_template(&t_sysact, TEMP_SYS);
	if((asyslogflag & 0x04))	read_template(&t_attack, TEMP_ATT);
	if((asyslogflag & 0x08))	read_template(&t_drops,  TEMP_DRP);
	if((asyslogflag & 0x10))	read_template(&t_notice, TEMP_NTC);
}
