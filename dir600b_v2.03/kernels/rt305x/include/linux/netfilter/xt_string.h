#ifndef _XT_STRING_H
#define _XT_STRING_H

#define XT_STRING_MAX_PATTERN_SIZE 128
#define XT_STRING_MAX_ALGO_NAME_SIZE 16

//joel add for block url
struct xt_string_info
{
	u_int16_t from_offset;
	u_int16_t to_offset;
	char	  algo[XT_STRING_MAX_ALGO_NAME_SIZE];
	char 	  pattern[XT_STRING_MAX_PATTERN_SIZE];
	u_int8_t  patlen;
	u_int8_t  invert;
    u_int8_t flags;  
	struct ts_config __attribute__((aligned(8))) *config;
};

enum ipt_string_flags
{
    IPT_GEN_STRING,
    IPT_URL_STRING,
    IPT_DNS_STRING,
    IPT_HTTP_REQ
};

#endif /*_XT_STRING_H*/
