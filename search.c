/******************************************************
 *         Pragram Name: search.c                  *
 *         Authore: Junhong Chen                      *
 *         Course: CS720/820                          *
 *         Assignment No.: 2                          *
 *         Purpose: BM Method.                        *
 ******************************************************/

#include "commonincludes.h"




void ShiftTable(DATA_t *data, int *Shift) {

  // Creat the shift table for the pattern string
  int length = strlen(data->pattern);
  int i, count  = 0;
  char *pp = data->pattern;

  // initialize the shift for all character to length
  //sf_len is define in header file as the max sizeof the shifttable
  for (i = 0; i < CHLEN; i++)
    {
      *(Shift + i) = length;
    }

  while (pp < data->pattern +length -1)
    { 
      if (data->flag.i == 0){ /* this is casesensitive*/
	*(Shift + (unsigned char)*pp++) = length - count -1;	
      }else{ /*this is caseinsensitive */
	char temp;
	temp = tolower(*pp++);
	*(Shift + (unsigned char)temp) = length - count -1;	
      }//endif
	
      count++;
    }//end while
      
}//shitftable




void strslc(char *str, char *p, int position, int length) {
  //extract a segament  in a line starting at a position

  int i;
  char *ptr;

  ptr = str;
  
  for (i = 0; i < position; i++)
    ptr++;
  for (i=0; i < length; i++){
    *(p + i) = *ptr;
    ptr++;
  }
  *(p + i) = '\0';
     
}



int stringcmp(char *s1, char *s2, int m, int i){
  //strcmp depends on if it is caseinsensitive

  int res;
  
  if (i == 1) 
    res = strncasecmp(s1,s2,m);
  else
    res = strncmp(s1,s2,m);

  return res;
}






int BM_Search(char *buf, DATA_t *data, int *Shift) {
  /* BM search */

  char *pattern = data->pattern;
  int n = strlen(buf);
  int m = strlen(pattern);
  


  if (data->flag.e == 1) { 
    /*ends at the end of file*/ 

    char p[m+1];
    strslc(buf,p,n-m,m);
    if (stringcmp(p,pattern,m,data->flag.i) == 0)
      return n-m;  
    else 
      return -1;
  }


 
  if (data->flag.b >= 0) { 
    /* starts at the starting of the line or the default search mode */
    
    int i = m-1,step = 0, steps = 0;
    while (i <= n-1)
      {
	//Check if it is caseinsensitive
	char p[m+1];
	steps += step;
	strslc(buf,p,steps,m);
	if (stringcmp(p,pattern,m,data->flag.i) == 0)
	  {
	    //if pattern matches, return	      
	    return steps;
	  } else {

	  //find the shift from the shfit table
	  step = Shift[(int)tolower((unsigned char)p[m-1])];
	  // make the shift
	  i += step;
	}
      }//end while
  }//end if


  return -1;
}


int search(char *buf, DATA_t *data, int *Shift) {

  int res;

  res = BM_Search(buf,data,Shift);

  //if -b and -e are both set
  if (data->flag.b == 1 && data->flag.e == 1){
    if (strlen(buf) != strlen(data->pattern))
      res = -1;
  }

  //if only -b is set
  if (data->flag.b == 1 && data->flag.e == 0){

    if (res != 0)
      res = -1;
  }

  // printf("Here v = %d : %d\n",data->flag.v,res);
  //if -v is set, then reverse the search result
  if (data->flag.v == 1){
    if (res != -1)
      res = -1;
    else
      res = 0;
  }
  return res;

}
