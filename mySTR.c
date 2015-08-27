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

void strncpy(char *dst, const char*src, size_t n)
{

while (n-- && (*dst++ = *src++));

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

int atoi(char *s){ 
	int sum = 0; 
	for(int i = 0;s[i] != '\0';i++){
		sum = sum*10+s[i]-'0'; 
	} 
	return sum; 
}


char* strcat(char *dest, const char *src)
{
    size_t i,j;
    for (i = 0; dest[i] != '\0'; i++)
        ;
    for (j = 0; src[j] != '\0'; j++)
        dest[i+j] = src[j];
    dest[i+j] = '\0';
    return dest;
}

#endif