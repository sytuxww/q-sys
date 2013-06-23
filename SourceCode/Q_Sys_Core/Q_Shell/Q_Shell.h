#ifndef QSYS_Q_SHELL_H  
#define QSYS_Q_SHELL_H

#define USE_Q_SHELL_FUNCTION 1

#if USE_Q_SHELL_FUNCTION

typedef const struct{
	const char*		name;		
	const char*		desc;		
	void           *addr;	
	const char*     typedesc;
}QSH_RECORD;

#define QSH_FUN_REG(name, desc)					                                \
static const   char  qsh_fun_##name##_name[]  = #name;				            \
static const   char  qsh_fun_##name##_desc[]  = desc;						    \
QSH_RECORD qsh_fun_##name##_record  __attribute__((section("qShellFunTab"))) =  \
{							                                                    \
	qsh_fun_##name##_name,	                                                    \
	qsh_fun_##name##_desc,	                                                    \
	(void *)&name,		                                                        \
	0                                                                           \
}

#define QSH_VAR_REG(name, desc,typedesc)					                   \
static const   char  qsh_var_##name##_name[] = #name;				           \
static const   char  qsh_var_##name##_desc[] = desc;				           \
static const   char  qsh_var_##name##_typedesc[] = typedesc;				   \
QSH_RECORD qsh_var_##name##_record  __attribute__((section("qShellVarTab"))) = \
{							                                                   \
	qsh_var_##name##_name,	                                                   \
	qsh_var_##name##_desc,	                                                   \
	(void *)&name,		                                                       \
	qsh_var_##name##_typedesc											       \
}

unsigned int Q_Sh_CmdHandler(unsigned int IfCtrl,char *CmdStr);

#else

#define QSH_FUN_REG(name, desc)
#define QSH_VAR_REG(name, desc)
unsigned int Q_Sh_CmdHandler(unsigned int IfCtrl,char *CmdStr);

#endif

#endif

