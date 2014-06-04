#include "commonincludes.h"


extern MESSAGE_ONE_t type1;

void PrintFlag(FILE *fp,usint flags, usint this_one, char *name){
  
  fprintf(fp,"%30s: 0x%02x\n",name, (flags & this_one) ? this_one : 0);

}


void PrintVal(FILE *fp,char *name, int value){

  fprintf(fp,"%30s: %d\n",name,value);

}




void PrintOpt(FILE *fp,MESSAGE_ONE_t type1){

  PrintFlag(fp,type1.flags,PROCESS_DOTNAME,"Option -a");
  PrintFlag(fp,type1.flags,STARTS_BEG,"Option -b");
  PrintFlag(fp,type1.flags,ENDS_END,"Option -e");
  PrintFlag(fp,type1.flags,NOFOLLOW_SYMLINK,"Option -f");
  PrintFlag(fp,type1.flags,CASE_INSENSITIVE,"Option -i");
  PrintFlag(fp,type1.flags,PRINT_REALPATH,"Option -p");
  PrintFlag(fp,type1.flags,QUIET,"Option -q");
  PrintFlag(fp,type1.flags,INVERT_OUTPUT,"Option -v");

  /*
    if (type1.d == -1)
    printf("%30s: %s\n","Value of -d","No Limit");
    else
  */
  PrintVal(fp,"Value of -d",type1.d);
  /*
    if (type1.l == -1)
    PrintVal("Value of -l",255);
    else
  */
  PrintVal(fp,"Value of -l",type1.l);
  /*
    if (type1.m == -1)
    printf("%30s: %s\n","Value of -m","No Limit");
    else
  */
  PrintVal(fp,"Value of -m",type1.m);
  /*
    if (type1.n == -1)
    printf("%30s: %s\n","Value of -n","Not Set");
    else
  */
  PrintVal(fp,"Value of -n",type1.n);
  /*
    if (type1.t == -1)
    printf("%30s: %s\n","Value of -t","No Limit");
    else
  */
  PrintVal(fp,"Value of -t",type1.t);

}



int trim_line(char *buf) { 
  int len;
   
  len = strlen(buf);
  if (len > 0 && buf[len-1] == '\n')
    buf[--len] = '\0';
  return len;
}



int scan_switch_num(int switch_char, int *result) { 

  int temp, retval = 0;
  char *ptr;
  int upper_lim;
   
  errno = 0;
  temp = strtol(optarg, &ptr, 10);
  if (errno != 0 || ptr == optarg || strspn(ptr, WHITE_SPACE) != strlen(ptr))
    {
      fprintf(stderr,"Illegal numerical value \"%s\" for switch -%c\n",optarg,switch_char);
      retval = -1;
      return retval;
    } else {
    switch (switch_char){
    case 'l':
      upper_lim = 4096;
      if (temp > 0 && temp < upper_lim + 1) {
	*result = temp;
      } else {
	fprintf(stderr,"Numerical value for switch -%c must be positive and less than %d\n",switch_char,upper_lim);
	retval = -1;
	return retval;
      }
      break;
    case 'm':
      if (temp > 0) {
	*result = temp;
      } else {
	fprintf(stderr,"Numerical value for switch -%c must be positive\n",switch_char);
	retval = -1;
	return retval;
      }
      break;
    case 'n':
      upper_lim = 16;
      if (temp > 0 && temp < upper_lim + 1) {
	*result = temp;
      } else {
	fprintf(stderr,"Numerical value for switch -%c be positive and less than %d\n",switch_char,upper_lim);
	retval = -1;
	return retval;
      }
      break;
    case 'd':
      if (temp >= 0) {
	*result = temp;
      } else {
	fprintf(stderr,"Numerical value for switch -%c must be non-negative\n",switch_char);
	retval = -1;
	return retval;
      }
      break;
    case 't':
      if (THMAX < 0){//no limit
	if (temp >= 0 ) {
	  *result = temp;
	} else {
	  fprintf(stderr,"Numerical value for switch -%c must be non-negative\n",switch_char);
	  retval = -1;
	  return retval;
	}
      } else {
	if (temp >= 0 && temp <= THMAX) {
	  *result = temp;
	} else {
	  fprintf(stderr,"Numerical value for switch -%c must be non-negative and less than %d\n",switch_char,THMAX);
	  retval = -1;
	  return retval;
	}
      }
      break;
    }
  }
  return EXIT_SUCCESS;
}





void print_realpath(char *fname) {
  int path_max;
  char *realptr, *memptr;

#if defined(PATH_MAX) && PATH_MAX != -1
  path_max = PATH_MAX;
#else
  errno = 0;
  if ((path_max = pathconf(fname, _PC_PATH_MAX)) == -1){
    if (errno != 0)
      perror(fname);
    /* error or unlimited, pick a big number */
    path_max = 8192;
  }
#endif
  memptr = malloc(path_max);
  if (memptr == NULL)
    printf("%14s: %s %s\n", "malloc()",
	   strerror(errno), fname);
  else {
    realptr = realpath(fname, memptr);
    if(realptr == NULL)
      printf("%14s: %s %s\n","realpath",
	     strerror(errno), fname);
    else {
      printf("%s: ",realptr);
    }
    free(memptr);
  }
}





void ClientParaInit(DATA_t *data, MESSAGE_ONE_t *type1){

  //initialize all the data in the structure data

  //init flag
  data->flag.b = 0;
  data->flag.e = 0;
  data->flag.i = 0;
  data->flag.v = 0;
  data->flag.p = 0;
  data->flag.h = 0;
  data->flag.a = 0;
  data->flag.f = 0;
  data->flag.q = 0;

  //init following arg
  data->num.l = 256;
  data->num.m = 0;
  data->num.n = 0; 
  data->num.d = 1000;//non-negative, defalut is a large number
  if (THMAX == -1)
    data->num.t = 1000;//non-negative, default is 1000 
  else 
    data->num.t = THMAX;

  /*
   * client message type 1
   */
  type1->flags = 0;
  type1->l = -1;//data->num.l;
  type1->m = -1;//data->num.m;
  type1->n = -1;//data->num.n;
  type1->d = -1;//data->num.d;
  type1->t = -1;//data->num.t;

  //init err status
  data->err.l = 0;
  data->err.m = 0;
  data->err.n = 0;
  data->err.d = 0; 
  data->err.t = 0;
  data->err.final = 0;

  //init THREAD_MAX;
}

void ServerParaInit(DATA_t *data, MESSAGE_ONE_t type1){

  //initialize all the data in the structure data

  //init flag
  data->flag.b = (type1.flags & STARTS_BEG)? 1 : 0;
  data->flag.e = (type1.flags & ENDS_END)? 1 : 0;
  data->flag.i = (type1.flags & CASE_INSENSITIVE)? 1 : 0;
  data->flag.v = (type1.flags & INVERT_OUTPUT)? 1 : 0;
  data->flag.p = (type1.flags & PRINT_REALPATH)? 1 : 0;
  data->flag.h = 0;
  data->flag.a = (type1.flags & PROCESS_DOTNAME)? 1 : 0;
  data->flag.f = (type1.flags & NOFOLLOW_SYMLINK)? 1 : 0;
  data->flag.q = (type1.flags & QUIET)? 1 : 0;

  /*
   * client message type 1
   */
  data->num.l = type1.l;
  if (type1.l == -1)
    data->num.l = 256;

  data->num.m = type1.m;
  if (type1.m == -1)
    data->num.m = 0;

  data->num.n = type1.n ;
  if (type1.n == -1)
    data->num.n = 0;

  data->num.d = type1.d;
  if (type1.d == -1)
    data->num.d = 10000;

  data->num.t = type1.t;
  if (type1.t == -1)
    data->num.t = 10000;
  //  data->pattern = type2;

}








void setopt(int argc, char *argv[], DATA_t *data, MESSAGE_ONE_t *type1){

  int c;

  opterr = 0;
  while ((c = getopt(argc,argv,OPTIONS)) != -1)
    {
      switch(c) {
      case 'h'://print help info
	printf("\nUsage: ./plcs [-habefipqv] [-d num][-l num] [-m num] [-t num] [-n num] [pattern] [- ...] [file ...]\n\n");
	printf(" -h: Print the help information.\n");
	printf(" -a: Process directories with names starting with.\n");
	printf(" -b: Select input line when pattern string starts at the beginning of the line.\n");
	printf(" -e: Select input line when pattern string ends at the end of the line.\n");
	printf(" -f: Do not follow symbolic link.\n");
	printf(" -i: The search for pattern string is caseinsensitive.\n");
	printf(" -p: Print the input file including the real path name to the standard output.\n     Ignore this when reading input files from standard input.\n");
	printf(" -q: Do nor wirte error message if directory cannot be processed.\n");
	printf(" -v: Print lines that are NOT selected.\n");
	printf(" -d num: Depth of directory of recursive search, num must be non-negative.\n");
	printf(" -l num: Break up lines into segaments, and num must be positive integer\n         and less than 4096.\n");
	printf(" -m num: Max line reads, and num must be positive integer.\n");
	printf(" -n num: Fromat output, and num must be positive integer and less than 16.\n");
	printf(" -t num: Active threads used to search, num must be non-negative.\n");
	
	data->flag.h = 1;
	//	break;
	exit(EXIT_SUCCESS);
      case 'b':     /* Select input line only if pattern string starts
		       at the beginning of the line.*/
	data->flag.b = 1;
	type1->flags |= STARTS_BEG; 
	break;
      case 'e':    /* Select input line only if pattern string ends
		      at the end of the line. */
	data->flag.e = 1;
	type1->flags |= ENDS_END; 
	break;
      case 'i':    /* if caseinsensitive */
	data->flag.i = 1;
	type1->flags |=  CASE_INSENSITIVE;
	break;
      case 'p':   /* Print input file including the path name at the begining */
	data->flag.p = 1;
	type1->flags |= PRINT_REALPATH;
	break;
      case 'v':
	data->flag.v = 1;
	type1->flags |= INVERT_OUTPUT;
	break;
      case 'a':
	data->flag.a = 1;
	type1->flags |= PROCESS_DOTNAME;
	break;
      case 'f':
	data->flag.f = 1;
	type1->flags |=  NOFOLLOW_SYMLINK;
	break;
      case 'q':
	data->flag.q = 1;
	type1->flags |=  QUIET;
	break;
      case 'l':
	data->err.l = scan_switch_num(c, &data->num.l);
	if (data->err.l != -1){
	  data->num.l += 1;
	  type1->l = data->num.l;
	} else {
	  data->err.final = -1;
	}
	break;
      case 'm':
	data->err.m = scan_switch_num(c, &data->num.m);
	type1->m = data->num.m;
	if (data->err.m == -1)
	  data->err.final = -1;	
	break;
      case 'n':
	data->err.n = scan_switch_num(c, &data->num.n);
	type1->n = data->num.n;
	if (data->err.n == -1)
	  data->err.final = -1;	
	break;
      case 'd':
	data->err.d = scan_switch_num(c, &data->num.d);
	type1->d = data->num.d;
	if (data->err.d == -1)
	  data->err.final = -1;	
	break;
      case 't':
	data->err.t = scan_switch_num(c, &data->num.t);
	type1->t = data->num.t;
	if (data->err.t == -1)
	  data->err.final = -1;	
	break;	
      case ':':
	fprintf(stderr,"Option -%c requires an argument.\n",optopt);
	data->err.final = -1;
	break;
      case '?':
	fprintf(stderr,"Illegal or Unknownn option '%c' \n",optopt);
	data->err.final = -1;
	break;
      }
    }


  if (argc - optind == 0 && data->err.final != -1 && data->flag.h == 0) {
    fprintf(stderr,"Missing search string.\n");
    data->err.final = -1;
  }

}


