#ifndef __MYSTR_H__
#define __MYSTR_H__

/*
return 0 for same(in "count" numbers)
return -1 for different
*/
int strncmp(const char *target,const char*source,uint32_t count)
{
	for(uint32_t i=0;i<count;i++){
		if(target[i]!=source[i]){
			return -1;
		}
	}
	return 0;
}

size_t strlen(const char *str){
	const char *s;
    for (s = str; *s; ++s);
    return (s - str);
}

void reverse(char *s){ 
	for(int i = 0,j = strlen(s)-1;i<j;i++,j--){ 
		int c = s[i];
		s[i] = s[j];
		s[j] = c; 
	}
}

void itoa(int n,char *s){
	int i = 0; 
	int flag = 1;

	if(n!=0){
			if (n<0){
 			n = -n; flag = 0;
 		} 
  		while(n != 0){
			s[i++] = n%10+'0';
 			n = n/10; 
		} 
 		if(!flag) s[i++] = '-';
 		s[i] = '\0'; 
 		reverse(s); 
	}else{
		s[0] = 0 + '0';
		s[1] = '\0';
	}

}

#endif