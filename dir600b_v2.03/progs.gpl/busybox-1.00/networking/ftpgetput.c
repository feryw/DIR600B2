/* vi: set sw=4 ts=4: */
/*
 * ftpget
 *
 * Mini implementation of FTP to retrieve a remote file.
 *
 * Copyright (C) 2002 Jeff Angielski, The PTR Group <jeff@theptrgroup.com>
 * Copyright (C) 2002 Glenn McGrath <bug1@iinet.net.au>
 *
 * Based on wget.c by Chip Rosenthal Covad Communications
 * <chip@laserlink.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 */

#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/stat.h>

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>

#include <netinet/in.h>
#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>
/*FW_log_20070412, start {, Added by Erick*/
/*ELBOX_PROGS_GPL_SYSLOGD*/
#include <syslog.h>
#include "../../../include/asyslog.h"
/*FW_log_20070412, End }, Added by Erick*/
#include "../../../include/elbox_config.h"/*ftp_tftp_FW_CG_20070125 Erick*/

#include "busybox.h"

typedef struct ftp_host_info_s {
	char *user;
	char *password;
	struct sockaddr_in *s_in;
} ftp_host_info_t;

static char verbose_flag = 0;
static char do_continue = 0;
/*ftp_tftp_FW_CG_20070125 Erick*//*start {*/
#if ELBOX_PROGS_GPL_TFTP_FTP_GET_PUT_FW_CONFIG
static char Get_FW = 0;
static char Put_CG = 0;
static char Get_CG = 0;
static char Get_ACL = 0;
static char Put_ACL = 0;

extern char g_signature[50];/*ftp_tftp_FW_CG_20070112 Erick*/
#define RGDBGET(x, y, args...) _cli_rgdb_get_(x, y, ##args)/*ftp_tftp_FW_CG_20070112 Erick*/
#define RGDBSET(x, y) _cli_rgdb_set_(x, y)
#define RGDBDEL(x) _cli_rgdb_del_(x)

char *ftp_buffer;
char *ftp_buffer_ptr;
int  ftp_len;
#define MAX_FW_BUFFER_SIZE 16*1024*1024 /*FW_log_20070412, Added by Erick*/ 
#define MAX_CFG_BUFFER_SIZE 1*1024*1024
#define MAX_ACL_BUFFER_SIZE 1024*32   //acl import outport  by traveller chen
#define Config_Path "/var/config.bin"
#endif
/*ftp_tftp_FW_CG_20070125 Erick*//*End }*/

/*Update_Bootloader_20070307, Builder, 20070307*/
/*+++*/

//acl import outport  by traveller chen start
#if  ELBOX_PROGS_GPL_TFTP_FTP_GET_PUT_FW_CONFIG

#define MAXACLNUM 256
#define MAXLINESIZ 18
static int aclmode=0; //o allow,1 reject

 void ftpSaveacl(char *bufout)
{
	char buf[128];
	char temp[128];
	int  len, i;
	FILE *fh;

   //band selection for dual band ap 2690 start
	int band =1;
    	char path[100],buff[30];
       memset(path,0,100);
	memset(buff,0,30);
	sprintf(path,"%s","/runtime/wireless/bandselection");
      	RGDBGET(buff,1, path);
	if(strlen(buff)!=0){
	band = atol(buff);
	}
   //band selection for dual band ap 2690 end
   
       memset(buf,0,128);
       memset(temp,0,128);
	RGDBGET(buf,128,"/wlan/inf:%d/acl/mode",band);
	if(!strcasecmp(buf,"0"))
	              sprintf(temp,"%s\n","OFF");
	else if(!strcasecmp(buf,"1"))
	              sprintf(temp,"%s\n","ALLOW");
	else if(!strcasecmp(buf,"2"))
	              sprintf(temp,"%s\n","DENY");
       else
	              sprintf(temp,"%s\n","OFF");

	strcat(bufout,temp);
        for(i=0;i<MAXACLNUM;i++)
      	{
         	memset(buf,0,128);
		memset(temp,0x0,128);
	      sprintf(buf,"/wlan/inf:%d/acl/mac:%d",band,(i+1));
	      RGDBGET(temp,128,buf); 
		  strcat(bufout,temp);
            if(strlen(temp))
            	{
            	   strcat(bufout,"\n");

            	}else
            	{
            	    break;
            	}
      	}
	
}



 int IsValidMac(char * buf,int linesize)
{
   int i=0;
   if(!buf||linesize!=17)
   	return -1;
   for(i=0;i<MAXLINESIZ-1;i++)
   {

	 if(!((buf[i]>='A'&&buf[i]<='F')||(buf[i]>='0'&&buf[i]<='9')||buf[i]==':'))
	  {
	 	return -1;
	  }
   	 if(i%3==2)
   	 {
   	       if(buf[i]!=':')
   	     	{
		   	return -1;
   	     	}
   	  }
	   else
	  {
	        if(!((buf[i]>='A'&&buf[i]<='F')||(buf[i]>='0'&&buf[i]<='9')))
	   	 {
		   	return -1;
	   	 }
	   }
   }
   return 0;
}

 int IsExist(char * mac,char * macList,int aclnum)
{
   int i=0;
   if(!mac||!macList)
   	return 0;
   for(i=0;i<aclnum*MAXLINESIZ;i=i+MAXLINESIZ)
   {
       if(!memcmp(&mac[0],&macList[i],MAXLINESIZ))
   	{
	     return 1;
   	}
   }
   return 0;
}
 int ReadACLFromFile(char * fileName,char *buffer)
{

   FILE *pFile;
   int aclnum=0;
   int state=1;
   char linebuf[MAXLINESIZ];
   int linesize=0;
   int totalnum=0; 
   char **arrAcl;
   int ignore=0;
   state = 1;
   arrAcl=(char**)buffer;
   if((pFile=fopen("/var/acl.tem","r")))
   	{
   	   while (state&&aclnum<MAXACLNUM&&totalnum<(MAXACLNUM*200)){
		int c;
		totalnum++;
		c = getc(pFile); 
		if (c== EOF) 
		{
		    ignore=0;
		    state = 0;
		    break;
		 }else if (c == 10||c ==  13)
		 {
		      if(linesize>0)
			{
			    linebuf[linesize]='\0';
			    if(linesize==3&&!memcmp(linebuf,"OFF",3))
			    {
			         aclmode=0;
		           }
			    else if (linesize==5&&!memcmp(linebuf,"ALLOW",5))
			    {
				  aclmode=1;
			    }
			     else if (linesize==4&&!memcmp(linebuf,"DENY",4))
			     {
			  	  aclmode=2;
			     }
			     else
			    {
			          if(IsValidMac(linebuf,linesize)!=0)
				   {
				       fclose(pFile);
				 	return -1;
				    }else
				    {
				         if(!IsExist(linebuf,buffer,aclnum))
				 	  {
					       memcpy(&buffer[aclnum*MAXLINESIZ],&linebuf[0],MAXLINESIZ);
				 	       aclnum++;
				 	    }
				 	}
				   }
				 memset(linebuf,0,sizeof(linebuf));
				
			     }
			linesize=0;
		       ignore=0;
		}
		else
		{
		    if(c == ';') ignore=1;   /*treat ';' as a commentary splitter*/
		    if(c=='-')c=':';
		    if(c>=97&&c<=122)c=c-32; //UpperCase	
		    if(!ignore&&c!=32&&linesize<(MAXLINESIZ-1))//ignore both space and the char(s) after ';' 
		    {
		        linebuf[linesize]=c;
			 linesize++;
		     }
		  }
   	  }
	
       fclose(pFile);
	if(aclnum>MAXACLNUM)
	   aclnum=-1;

	return aclnum;
   }
}


 int UpdateAclList(char * buffer,int aclnum)
{
   int i=0;
   char path[100],buff[30];
   int aclno=0;
   
   //band selection for dual band ap 2690 start
	int band =1;
       memset(path,0,100);
	memset(buff,0,30);
	sprintf(path,"%s","/runtime/wireless/bandselection");
      	RGDBGET(buff,1, path);
	if(strlen(buff)!=0){
	band = atol(buff);
	}
   //band selection for dual band ap 2690 end
 	
   if((!buffer)||aclnum<0||aclnum>MAXACLNUM){
   	return -1;
   	}
   memset(path,0,100);
   memset(buff,0,30);
   //clear the old acl list
      for(i=0;i<MAXACLNUM;i++)
      	{
      	     memset(path,0,100);
            memset(buff,0,30);
	     sprintf(path,"/wlan/inf:%d/acl/mac:%d",band,1);
	   // printf("path:%s\n",path);
      	     RGDBGET(buff,10, path);
	  //  printf("buff:%s\n",buff);
            if(strlen(buff))
            	{
            	  RGDBDEL(path);
            	}else
            	{
            		break;
            	}
      	}
   //set acl mode,0:off,1:allow,2:deny
   sprintf(path,"/wlan/inf:%d/acl/mode",band);
   sprintf(buff,"%d",aclmode);
   RGDBSET(path,buff);
 //set acl list
   for(i=0;i<aclnum*MAXLINESIZ;i=i+MAXLINESIZ)
   	{		
            memset(path,0,100);
            memset(buff,0,30);
            sprintf(path,"/wlan/inf:%d/acl/mac:%d",band,(aclno+1));
            sprintf(buff,"%s",(char*)&buffer[i]);
            RGDBSET(path,buff);
	     aclno++;
	}
		
 memset(path,0,100);
 memset(buff,0,30);
 sprintf(path,"/runtime/sys/fw_size");
 sprintf(buff,"%d",(aclno+1));
  RGDBSET(path,buff);
  return 0;
}


 int ftpUpload_acl(char *input,int length)
{
	char *pInput=input;
	FILE *pFile;
	int filelen = length;
       char* aclFileName="/var/acl.tem";
       char* buffer=NULL;
	int aclnum=0;
//2009_07_02 sandy++++++
#ifdef CONFIG_AP_NEAP_AP_ARRAY	
	int cfg_version_buf;
	char temp_buf[128];
#endif	
//2009_07_02 sandy-----
       if((pFile=fopen(aclFileName,"w")))
	{
		fwrite(pInput, 1, filelen, pFile);
		fclose(pFile);
		//Read setting to buffer;
		buffer = malloc(MAXACLNUM*MAXLINESIZ);
              if(buffer == NULL)
                   goto error;
		if((aclnum=ReadACLFromFile(aclFileName,buffer))<0)
		     {
		      goto error;
		     }
		if(UpdateAclList(buffer,aclnum)!=0)
		    {
	              goto error;
		    }
		if(buffer) free(buffer);
		unlink(aclFileName);
		//rlt_page(p, "sys_stunnel_process.php");
		//RGDBSET( "/runtime/web/redirect_next_page","sys_stunnel_process.php");
		//redirect_page(p, "/www/sys/redirectlink.php");
		      
		//sync_time();
		//upload_file.flag=0;
		//upload_file.uptime=current_uptime;
//-------------------------------------------
//ap array config version conuter 2009_07_02 sandy+++++++		
#ifdef CONFIG_AP_NEAP_AP_ARRAY
//	status_buf =0; //2009_9_2 sandy
//        RGDBGET(temp_buf,sizeof(temp_buf), "/wlan/inf:1/APARRAY_enable");
//	status_buf = atol(temp_buf);
//	if (status_buf == 1)
//	{	
				memset(temp_buf, 0x0, sizeof(temp_buf));
				RGDBGET(temp_buf,sizeof(temp_buf), "/wlan/inf:1/aparray_cfg_version");
				cfg_version_buf =0; 
				cfg_version_buf = atol(temp_buf);
				cfg_version_buf++;
				memset(temp_buf, 0x0, sizeof(temp_buf));
				sprintf( temp_buf, "%d", cfg_version_buf );
				RGDBSET( "/wlan/inf:1/aparray_cfg_version",temp_buf);
//	}		
#endif	
//-------------------------------------------

		return 0;
	 }


	error:
	free(buffer);
	//unlink(aclFileName);
	//rlt_page(p, "sys_stunnel_error.php");
	//RGDBSET( "/runtime/web/redirect_next_page","sys_stunnel_error.php");
	//redirect_page(p, "/www/sys/redirectlink.php");
	return -1;
}


#endif
//acl import outport  by traveller chen end
int bootloader_main(int argc, char **argv)
{
    char *srcf, *dstf = "/dev/mtdblock/3";
    struct stat src_stat;
    FILE *srcfd, *dstfd;
    
    if (argc!=2){
        bb_show_usage();
        return -1;
    }
        
    srcf = argv[1];

    if (lstat(srcf, &src_stat) < 0) {
        if (errno != ENOENT) {
            bb_perror_msg("unable to stat `%s'", srcf);
        }else{
            bb_perror_msg("%s", srcf);
        }
        return -1;
    }
    
    if(copy_file(srcf, dstf, 2)!=0){
        return -1;
    }else{
        if(remove_file(srcf, 2)!=0)
            return -1;
    }

    return 0;
}
/*+++*/
/*Update_Bootloader_20070307, Builder, 20070307*/

static int ftpcmd(const char *s1, const char *s2, FILE *stream, char *buf)
{
	if (verbose_flag) {
		bb_error_msg("cmd %s%s", s1, s2);
	}

	if (s1) {
		if (s2) {
			fprintf(stream, "%s%s\r\n", s1, s2);/*ftp_tftp_FW_CG_20070125 Erick*/
		} else {
			fprintf(stream, "%s\r\n", s1);/*ftp_tftp_FW_CG_20070125 Erick*/
		}
	}
	do {
		char *buf_ptr;

		if (fgets(buf, 510, stream) == NULL) {
			bb_perror_msg_and_die("fgets()");
		}
		buf_ptr = strstr(buf, "\r\n");
		if (buf_ptr) {
			*buf_ptr = '\0';
		}
	} while (! isdigit(buf[0]) || buf[3] != ' ');

	return atoi(buf);
}

static int xconnect_ftpdata(ftp_host_info_t *server, const char *buf)
{
	char *buf_ptr;
	unsigned short port_num;

	buf_ptr = strrchr(buf, ',');
	*buf_ptr = '\0';
	port_num = atoi(buf_ptr + 1);

	buf_ptr = strrchr(buf, ',');
	*buf_ptr = '\0';
	port_num += atoi(buf_ptr + 1) * 256;

	server->s_in->sin_port=htons(port_num);
	return(xconnect(server->s_in));
}

static FILE *ftp_login(ftp_host_info_t *server)
{
	FILE *control_stream;
	char buf[512];

	/* Connect to the command socket */
	control_stream = fdopen(xconnect(server->s_in), "r+");
	if (control_stream == NULL) {
		bb_perror_msg_and_die("Couldnt open control stream");
	}

	if (ftpcmd(NULL, NULL, control_stream, buf) != 220) {
		bb_error_msg_and_die("%s", buf + 4);
	}

	/*  Login to the server */
	switch (ftpcmd("USER ", server->user, control_stream, buf)) {
	case 230:
		break;
	case 331:
		if (ftpcmd("PASS ", server->password, control_stream, buf) != 230) {
			bb_error_msg_and_die("PASS error: %s", buf + 4);
		}
		break;
	default:
		bb_error_msg_and_die("USER error: %s", buf + 4);
	}

	ftpcmd("TYPE I", NULL, control_stream, buf);

	return(control_stream);
}

#ifdef CONFIG_FTPGET
static int ftp_recieve(ftp_host_info_t *server, FILE *control_stream,
		const char *local_path, char *server_path)
{
	char buf[512];
	off_t filesize = 0;
	int fd_data;
	int fd_local = -1;
	off_t beg_range = 0;
#if ELBOX_PROGS_GPL_TFTP_FTP_GET_PUT_FW_CONFIG /*ftp_tftp_FW_CG_20070125 Erick*/
	char sbuf[50];
#endif

	/* Connect to the data socket */
	if (ftpcmd("PASV", NULL, control_stream, buf) != 227) {
		bb_error_msg_and_die("PASV error: %s", buf + 4);
	}
	fd_data = xconnect_ftpdata(server, buf);

	if (ftpcmd("SIZE ", server_path, control_stream, buf) == 213) {
		unsigned long value=filesize;
		if (safe_strtoul(buf + 4, &value))
			bb_error_msg_and_die("SIZE error: %s", buf + 4);
		filesize = value;
	} else
	{
		filesize = -1;
		do_continue = 0;
	}
/*ftp_tftp_FW_CG_20070125 Erick*//*start {*/
#if ELBOX_PROGS_GPL_TFTP_FTP_GET_PUT_FW_CONFIG
    if(Get_CG)
    {
	        /*The configuration file will be saved to /var/config.bin, 
	        and we will parse it after we get it by using ftp.*/
	        sprintf(sbuf,Config_Path);
	        //printf("set local_path to /var/config.bin\n");
	        local_path = sbuf;
	}
    if(!Get_CG && !Get_FW && ! Get_ACL)/*ftp_tftp_FW_CG_200609028 Erick*/
#endif
/*ftp_tftp_FW_CG_20070125 Erick*//*End }*/
	if ((local_path[0] == '-') && (local_path[1] == '\0')) {
		fd_local = STDOUT_FILENO;
		do_continue = 0;
	}

/*ftp_tftp_FW_CG_20070125 Erick*//*start {*/
#if ELBOX_PROGS_GPL_TFTP_FTP_GET_PUT_FW_CONFIG
    if(Get_FW || Get_CG ||Get_ACL)
    {
    	
    	
        if(Get_FW){
        		system("cp /usr/sbin/reboot /var/run/reboot");                      
              system("rgdb -i -s /runtime/update/fwimage/status 1");
	        ftp_buffer=malloc(MAX_FW_BUFFER_SIZE);
	      }else if(Get_ACL){
	        ftp_buffer=malloc(MAX_ACL_BUFFER_SIZE);

	      	}else{
	        ftp_buffer=malloc(MAX_CFG_BUFFER_SIZE);}
	    
	    
        if(ftp_buffer)
        {
            ftp_buffer_ptr=ftp_buffer;
            ftp_len=0;
           // printf("ftp allocate download_buffer->OK!\n");
        }
        else
        {
           // printf("ftp allocate download_buffer->Fail!\n");
            exit(EXIT_FAILURE);
        } 
        
    }
#endif
/*ftp_tftp_FW_CG_20070125 Erick*//*End }*/
	if (do_continue) {
		struct stat sbuf;
		if (lstat(local_path, &sbuf) < 0) {
			bb_perror_msg_and_die("fstat()");
		}
		if (sbuf.st_size > 0) {
			beg_range = sbuf.st_size;
		} else {
			do_continue = 0;
		}
	}

	if (do_continue) {
		sprintf(buf, "REST %ld", (long)beg_range);
		if (ftpcmd(buf, NULL, control_stream, buf) != 350) {
			do_continue = 0;
		} else {
			filesize -= beg_range;
		}
	}

	if (ftpcmd("RETR ", server_path, control_stream, buf) > 150) {
		bb_error_msg_and_die("RETR error: %s", buf + 4);
	}

	/* only make a local file if we know that one exists on the remote server */
#if ELBOX_PROGS_GPL_TFTP_FTP_GET_PUT_FW_CONFIG/*ftp_tftp_FW_CG_20070125 Erick*/
	if (fd_local == -1 && !Get_FW && !Get_ACL) {
#else
	if (fd_local == -1) {
#endif
		if (do_continue) {
			fd_local = bb_xopen(local_path, O_APPEND | O_WRONLY);
		} else {
			fd_local = bb_xopen(local_path, O_CREAT | O_TRUNC | O_WRONLY);
		}
	}


#if ELBOX_PROGS_GPL_TFTP_FTP_GET_PUT_FW_CONFIG/*ftp_tftp_FW_CG_20070503 Erick*/
	/* Copy the file */
	if (filesize != -1 && (!Get_FW) && (!Get_CG) &&(!Get_ACL)) {
#else
	/* Copy the file */
	if (filesize != -1) {
#endif	    
		if (-1 == bb_copyfd_size(fd_data, fd_local, filesize))
		exit(EXIT_FAILURE);
	} else {
#if ELBOX_PROGS_GPL_TFTP_FTP_GET_PUT_FW_CONFIG/*ftp_tftp_FW_CG_20070125 Erick*/
	    if (Get_FW)
	    {
	        if (-1 == (ftp_len=bb_copyfd2buff_eof(fd_data, ftp_buffer_ptr, MAX_FW_BUFFER_SIZE)))
	        {
/*FW_log_20070412, start {, Added by Erick*/   
                 bb_error_msg("Fail to get the file, please check the IP address and check the file name.");	            
#if ELBOX_PROGS_GPL_SYSLOGD  
                /*ftp_tftp_log_20070427*/
                 syslog(ALOG_AP_SYSACT|LOG_WARNING,"[SYSACT]Fail to get the file, please check the IP address and check the file name.");
#endif 
/*FW_log_20070412, End }, Added by Erick*/ 	            
		exit(EXIT_FAILURE);
	}
	    }else if (Get_CG)
	    {
	        if (-1 == (ftp_len=bb_copyfd2buff_eof(fd_data, ftp_buffer_ptr, MAX_CFG_BUFFER_SIZE)))
	        {
	           /*FW_log_20070412, start {, Added by Erick*/   
               bb_error_msg("Fail to get the file.");	
               /*Remove local file /var/config.bin*/
	           unlink(Config_Path); 
	            /*FW_log_20070412, End }, Added by Erick*/      
		    exit(EXIT_FAILURE);
		    }
	    }else if(Get_ACL){
	        if (-1 == (ftp_len=bb_copyfd2buff_eof(fd_data, ftp_buffer_ptr, MAX_ACL_BUFFER_SIZE)))
	        {

		exit(EXIT_FAILURE);

	        }

	    }else
#endif /*ftp_tftp_FW_CG_20070125 Erick*/
	    {   
                    if (-1 == bb_copyfd_eof(fd_data, fd_local))
		    exit(EXIT_FAILURE);
	    }
	}
	/* close it all down */
	close(fd_data);
	if (ftpcmd(NULL, NULL, control_stream, buf) != 226) {
		bb_error_msg_and_die("ftp error: %s", buf + 4);
	}
	ftpcmd("QUIT", NULL, control_stream, buf);

/*ftp_tftp_FW_CG_20070125 Erick*//*start {*/
#if ELBOX_PROGS_GPL_TFTP_FTP_GET_PUT_FW_CONFIG
    if(Get_FW)
    {

#ifdef ELBOX_PROGS_PRIV_CLI_SUPER
                    {
                        char xmldb_buff[10];
                        
                        memset(xmldb_buff, 0x0, sizeof(xmldb_buff));
                        RGDBGET(xmldb_buff, 10, "/runtime/sys/super");
                        //xmldbc_get_wb(NULL, 0, "/runtime/sys/super", xmldb_buff, sizeof(xmldb_buff)-1);
                        if(atoi(xmldb_buff)==1){
#ifdef ELBOX_FIRMWARE_HEADER_VERSION
                            if(ELBOX_FIRMWARE_HEADER_VERSION == 3){
                                burn_image(ftp_buffer, ftp_len);
                                return(EXIT_SUCCESS);
                            }else if(ELBOX_FIRMWARE_HEADER_VERSION == 2){
                                v2_burn_image(ftp_buffer, ftp_len);
                                return(EXIT_SUCCESS);
                            }
#else
                            v2_burn_image(ftp_buffer, ftp_len);
                            return(EXIT_SUCCESS);
#endif /*ELBOX_FIRMWARE_HEADER_VERSION*/
                        }
                    }
#endif /*ELBOX_PROGS_PRIV_CLI_SUPER*/
#ifdef ELBOX_FIRMWARE_HEADER_VERSION
                    if(ELBOX_FIRMWARE_HEADER_VERSION == 3){
                        if (v3_image_check(ftp_buffer, ftp_len)==0)
                        {
                            v3_burn_image(ftp_buffer, ftp_len);
                        }
                    }else if(ELBOX_FIRMWARE_HEADER_VERSION == 2){
                        if (v2_image_check(ftp_buffer, ftp_len)==0)
                        {
                            v2_burn_image(ftp_buffer, ftp_len);
                        }                        
                    }
#else
                    if (v2_image_check(ftp_buffer, ftp_len)==0)
                    {
                        v2_burn_image(ftp_buffer, ftp_len);
                    }
                    else
                    {

                    }
#endif /*ELBOX_FIRMWARE_HEADER_VERSION*/
    }
    

    if(Get_CG)
	{
	    
	        memset(g_signature,0,50);
                RGDBGET(g_signature, 50, "/runtime/layout/image_sign");

	        ftp_buffer_ptr=ftp_buffer;
	        ftp_buffer_ptr+=strlen(g_signature)+1;
	        ftp_len-=strlen(g_signature)+1;
	        
	        if(write(fd_local,ftp_buffer_ptr,ftp_len)<=0)
	        {
	            bb_error_msg("Write config to dev->fail");
	        }
	        else
	        {
	            system("/etc/scripts/misc/profile.sh reset /var/config.bin");
	            system("/etc/scripts/misc/profile.sh put");/*Save_config_20070426 Erick*/
	            /*FW_log_20070412, Added by Erick*/                 	
                bb_error_msg("Update configuraion file successfully!");
            	/*phelpsll add update success message. 2009/08/11*/
                bb_error_msg("Please reboot device!");
	        }

	        close(fd_local);
	        /*Remove local file /var/config.bin*/
	        unlink(Config_Path);
	}		

	if(Get_ACL){
		ftpUpload_acl(ftp_buffer,ftp_len);
		}
    if(ftp_buffer)
    {
        free(ftp_buffer);
        ftp_buffer=NULL;
       // printf("Free download_buffer\n");
    }
#endif
/*ftp_tftp_FW_CG_20070125 Erick*//*End }*/	
	return(EXIT_SUCCESS);
}
#endif

#ifdef CONFIG_FTPPUT
static int ftp_send(ftp_host_info_t *server, FILE *control_stream,
		const char *server_path, char *local_path)
{
	struct stat sbuf;
	char buf[512];
	int fd_data;
	int fd_local;
	int response;
#if ELBOX_PROGS_GPL_TFTP_FTP_GET_PUT_FW_CONFIG /*ftp_tftp_FW_CG_20070125 Erick*/
	char ssbuf[50]; 
#endif
	/*  Connect to the data socket */
	if (ftpcmd("PASV", NULL, control_stream, buf) != 227) {
		bb_error_msg_and_die("PASV error: %s", buf + 4);
	}
	fd_data = xconnect_ftpdata(server, buf);
/*ftp_tftp_FW_CG_200609028 Erick*//*start {*/
/*	if (ftpcmd("CWD ", server_path, control_stream, buf) != 250) {
		bb_error_msg_and_die("CWD error: %s", buf + 4);
	}*/
#if ELBOX_PROGS_GPL_TFTP_FTP_GET_PUT_FW_CONFIG /*ftp_tftp_FW_CG_20070125 Erick*//*start {*/
    if(Put_CG)
    {
	        /*To generate the configuration file to put to PC*/
	        //printf("get config from flash\n");
	        system("/usr/sbin/sys -s configsave");
	        /*The file will be /var/config.bin*/
	        //printf("set local file to /var/config.bin\n");
	        sprintf(ssbuf,Config_Path);
	        local_path = ssbuf;
	}
#endif
/*ftp_tftp_FW_CG_20070125 Erick*//*End }*/
	/* get the local file */
#if ELBOX_PROGS_GPL_TFTP_FTP_GET_PUT_FW_CONFIG /*ftp_tftp_FW_CG_20070125 Erick*/
	if ((local_path[0] == '-') && (local_path[1] == '\0') && !Put_CG) {/*ftp_tftp_FW_CG_200609028 Erick*/
#else
	if ((local_path[0] == '-') && (local_path[1] == '\0')) {
#endif
		fd_local = STDIN_FILENO;
	} else {
#if ELBOX_PROGS_GPL_TFTP_FTP_GET_PUT_FW_CONFIG //by traveller chen
		if(Put_CG){
		fd_local = bb_xopen(local_path, O_RDONLY);
		fstat(fd_local, &sbuf);
			}
#else
		fd_local = bb_xopen(local_path, O_RDONLY);
		fstat(fd_local, &sbuf);
#endif
/*ftp_tftp_FW_CG_20070125 Erick*//*Disabled by Erick*//*start {*/
		/*sprintf(buf, "ALLO %lu", (unsigned long)sbuf.st_size);
		response = ftpcmd(buf, NULL, control_stream, buf);
		switch (response) {
		case 200:
		case 202:
			break;
		default:
			close(fd_local);
			bb_error_msg_and_die("ALLO error: %s", buf + 4);
			break;
		}*//*ftp_tftp_FW_CG_200609028 Erick*//*End }*/
		}
	response = ftpcmd("STOR ", server_path, control_stream, buf);/*ftp_tftp_FW_CG_20070125 Erick*/
	switch (response) {
	case 125:
	case 150:
		break;
	default:
		close(fd_local);
#if ELBOX_PROGS_GPL_TFTP_FTP_GET_PUT_FW_CONFIG /*FW_log_20070412,start{, Added by Erick*/ 
        if(Put_CG)
        {		
            /*Remove local file /var/config.bin*/
	        unlink(local_path);
	        bb_error_msg("Put configuraion file fail!");
        }
#endif /*FW_log_20070412,End}, Added by Erick*/
		bb_error_msg_and_die("STOR error: %s", buf + 4);
	}
#if ELBOX_PROGS_GPL_TFTP_FTP_GET_PUT_FW_CONFIG //by traveller chen
	/* transfer the file  */
	if(Put_CG){
		if (bb_copyfd_eof(fd_local, fd_data) == -1) {
			exit(EXIT_FAILURE);
		}
	}

	//traveller add for acl transfer
	if(Put_ACL){
		char acl[40000];
		memset(acl,0x0,sizeof(acl));
		ftpSaveacl(acl);
		if (bb_full_write(fd_data,acl,strlen(acl)) != strlen(acl) ) {
			exit(EXIT_FAILURE);
		}
	}
#else 
	/* transfer the file  */
	if (bb_copyfd_eof(fd_local, fd_data) == -1) {
		exit(EXIT_FAILURE);
	}

#endif

	/* close it all down */
	close(fd_data);
	if (ftpcmd(NULL, NULL, control_stream, buf) != 226) {
		bb_error_msg_and_die("error: %s", buf + 4);
	}
	ftpcmd("QUIT", NULL, control_stream, buf);
/*ftp_tftp_FW_CG_20070125 Erick*//*start {*/
#if ELBOX_PROGS_GPL_TFTP_FTP_GET_PUT_FW_CONFIG 
    if(Put_CG)
    {
        close(fd_local);
	    /*Remove local file /var/config.bin*/
	    unlink(local_path);
	    /*FW_log_20070412, Added by Erick*/ 
	    bb_error_msg("Put configuraion file successfully!");
    }
#endif    
/*ftp_tftp_FW_CG_20070125 Erick*//*End }*/
	return(EXIT_SUCCESS);
}
#endif

#define FTPGETPUT_OPT_CONTINUE	1
/*ftp_tftp_FW_CG_20070125 Erick*//*start {*/
#define FTPGETPUT_OPT_FW     	2
#define FTPGETPUT_OPT_PUT_CG	4
#define FTPGETPUT_OPT_GET_CG	8
#define FTPGETPUT_OPT_VERBOSE	0x10
#define FTPGETPUT_OPT_USER	0x20
#define FTPGETPUT_OPT_PASSWORD	0x40
#define FTPGETPUT_OPT_PORT	0x80
#define FTPGETPUT_OPT_GET_ACL 0x100
#define FTPGETPUT_OPT_PUT_ACL 0x200
/*ftp_tftp_FW_CG_20070125 Erick*//*End }*/

static const struct option ftpgetput_long_options[] = {
	{"continue", 1, NULL, 'c'},
#if ELBOX_PROGS_GPL_TFTP_FTP_GET_PUT_FW_CONFIG /*ftp_tftp_FW_CG_20070125 Erick*//*start {*/
	{"firmware", 1, NULL, 'f'},
	{"put_config", 1, NULL, 't'},
	{"get_config", 1, NULL, 'd'},
#endif
	{"verbose", 0, NULL, 'v'},
	{"username", 1, NULL, 'u'},
	{"password", 1, NULL, 'p'},
	{"port", 1, NULL, 'P'},
	{0, 0, 0, 0}
};

int ftpgetput_main(int argc, char **argv)
{
	/* content-length of the file */
	unsigned long opt;
	char *port = "ftp";

	/* socket to ftp server */
	FILE *control_stream;
	struct sockaddr_in s_in;

	/* continue a prev transfer (-c) */
	ftp_host_info_t *server;

	int (*ftp_action)(ftp_host_info_t *, FILE *, const char *, char *) = NULL;

	/* Check to see if the command is ftpget or ftput */
#ifdef CONFIG_FTPPUT
# ifdef CONFIG_FTPGET
	if (bb_applet_name[3] == 'p') {
		ftp_action = ftp_send;
	}
# else
	ftp_action = ftp_send;
# endif
#endif
#ifdef CONFIG_FTPGET
# ifdef CONFIG_FTPPUT
	if (bb_applet_name[3] == 'g') {
		ftp_action = ftp_recieve;
	}
# else
	ftp_action = ftp_recieve;
# endif
#endif

	/* Set default values */
	server = xmalloc(sizeof(ftp_host_info_t));
	server->user = "anonymous";
	server->password = "busybox@";
	verbose_flag = 0;

	/*
	 * Decipher the command line
	 */
	bb_applet_long_options = ftpgetput_long_options;
#if ELBOX_PROGS_GPL_TFTP_FTP_GET_PUT_FW_CONFIG /*ftp_tftp_FW_CG_20070125 Erick*//*start {*/
	opt = bb_getopt_ulflags(argc, argv, "cftdvu:p:P:mn", &server->user, &server->password, &port);/*ftp_tftp_FW_CG_200609028 Erick*/

		Get_FW = 0;
		Put_CG = 0;
		Get_CG = 0;
		Get_ACL=0;
		Put_ACL= 0;
	
	if (opt & FTPGETPUT_OPT_FW) {
		Get_FW = 1;
	}
	if (opt & FTPGETPUT_OPT_PUT_CG) {
		Put_CG = 1;
	}
	if (opt & FTPGETPUT_OPT_GET_CG) {
		Get_CG = 1;
	}
	if(opt & FTPGETPUT_OPT_GET_ACL){
		Get_ACL =1;	
	}
	if(opt & FTPGETPUT_OPT_PUT_ACL){
		Put_ACL =1;	
	}	

/*ftp_tftp_FW_CG_20070125 Erick*//*End }*/
#else
	opt = bb_getopt_ulflags(argc, argv, "cvu:p:P:", &server->user, &server->password, &port);
#endif

	/* Process the non-option command line arguments */
	if (argc - optind != 3) {
#if ELBOX_PROGS_GPL_TFTP_FTP_GET_PUT_FW_CONFIG /*ftp_tftp_FW_CG_20070125 Erick*//*start {*/
	    if((Get_FW || Get_CG || Put_CG || Get_ACL || Put_ACL) &&(argc - optind != 2))
		bb_show_usage();
		else if(!(Get_FW || Get_CG || Put_CG || Get_ACL || Put_ACL))
#endif
/*ftp_tftp_FW_CG_20070125 Erick*//*End }*/
		bb_show_usage();
	}

	if (opt & FTPGETPUT_OPT_CONTINUE) {
		do_continue = 1;
	}
	if (opt & FTPGETPUT_OPT_VERBOSE) {
		verbose_flag = 1;
	}

	/* We want to do exactly _one_ DNS lookup, since some
	 * sites (i.e. ftp.us.debian.org) use round-robin DNS
	 * and we want to connect to only one IP... */
	server->s_in = &s_in;
	bb_lookup_host(&s_in, argv[optind]);
	s_in.sin_port = bb_lookup_port(port, "tcp", 21);
	if (verbose_flag) {
		printf("Connecting to %s[%s]:%d\n",
				argv[optind], inet_ntoa(s_in.sin_addr), ntohs(s_in.sin_port));
	}

	/*  Connect/Setup/Configure the FTP session */
	control_stream = ftp_login(server);
#if ELBOX_PROGS_GPL_TFTP_FTP_GET_PUT_FW_CONFIG /*ftp_tftp_FW_CG_20070125 Erick*//*start {*/
    if((Get_FW || Get_CG || Put_CG || Get_ACL || Put_ACL))
    return(ftp_action(server, control_stream, argv[optind + 1], argv[optind + 1]));
    else
#endif
/*ftp_tftp_FW_CG_20070125 Erick*//*End }*/
	return(ftp_action(server, control_stream, argv[optind + 1], argv[optind + 2]));
}

/*
Local Variables:
c-file-style: "linux"
c-basic-offset: 4
tab-width: 4
End:
*/
