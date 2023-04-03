char strlen(char* str)
{
	const char *s;
	for (s = str; *s; ++s) ;
	return s - str;
}

char strpos(char* search, char* content, char start)
{
	int i, j, check, result = -1;
	int len_search = strlen(search);
	int len_content = strlen(content);  
	i = start;
	if (len_search>len_content || *search == '\0' || i>(len_content - 1)) return result;

	for(; i < len_content; ++i)
	{
		check = 0;
		for(j = 0; j < len_search; ++j)
			if(content[i + j] == search[j])
				++check;
             
		if(check == len_search)
		{
			result = i;
			break;
		}

	}
	return result;
}

//radix unused, always 10
void itoa(int value, char *sp, char radix)
{
	char tmp[16];// be careful with the length of the buffer
	char *tp = tmp;
	int i;
	unsigned v;

	int sign = value < 0;    
	if (sign)
		v = -value;
	else
		v = (unsigned)value;

	while (v || tp == tmp)
	{
		i = v % radix;
		v /= radix; 
		if (i < 10)
			*tp++ = i+'0';
		else
			*tp++ = i + 'a' - 10;
	}

   

	if (sign) 
		*sp++ = '-';
    

	while (tp > tmp)
		*sp++ = *--tp;
}

int atoi(char* str) 
{
	int k = 0;
	while (*str) 
	{
		k = (k << 3) + (k << 1) + (*str) - '0';
		++str;
	}
	return k;
}

char* strcat(char *dest, const char *src)
{
	char *rdest = dest;

	while (*dest)	dest++;
	while (*dest++ = *src++) ;
	
	return rdest;
}

int strcmp(const char* str1, const char* str2)
{
	while(*str1 && (*str1 == *str2))
	{
		++str1;
		++str2;
	}
	return *(const unsigned char*)str1 - *(const unsigned char*)str2;
}

void strncpy(char* dst, const char* src, char n )
{
	char i = 0;
	while(i++ != n && (*dst++ = *src++));
}

const char* strcpy(char* dst, const char* src)
{
	const char *start = src;
	while(*dst++ = *src++) ;
	return start;
}

char upcase(char c)
{	
	return (c >= 'a' && c <= 'z') ? ('A' + (c - 'a')) : c;
}	

char downcase(char c) 
{
	return (c >= 'A' && c <= 'Z') ? ('a' + (c - 'A')) : c;
}

const char *strchr(const char *s, const char ch)
{
	for (; *s != '\0'; ++s)
  		if (*s == ch)
    		return s;

	return 0;
}


char isalpha(const char mark)
{
	return 	(mark >= 0x41) && (mark <= 0x5A) ||
			(mark >= 0x61) && (mark <= 0x7A); 
}

int pow(int base, int ex)
{
	int result = base;
	for(int i = 0; i < ex; ++i)
		result *= 10;
	return result;
}

float str_to_float(char *arr)
{
    int i,j,flag;
    float val;
    char c;
    i=0;
    j=0;
    val=0;
    flag=0;
    while ((c = *(arr+i))!='\0')
    {
//      if ((c<'0')||(c>'9')) return 0;
        if (c!='.')
        {
            val =(val*10)+(c-'0');
            if (flag == 1)
	            --j;
        }
        if (c == '.')
        { 
        	if (flag == 1) 
        		return 0; 
        	flag=1;
        }
        ++i;
    }
    val = val*pow(10, j);
    return val;
}

int itoa_s(int value, char *buf) 
{
        int index = 0;
        int i = value % 10;
        if (value >= 10) {
                index += itoa_s(value / 10, buf);
        }
        buf[index] = i+0x30;
        index++;
        return index;
}

char* ftoa(float value, int decimals, char* buf) 
{
	int d;	
	int index = 0;
	// Handle negative values
	if (value < 0) {
	       buf[index] = '-';
	       index++;
	       value = -value;
	}

	// Rounding
	float rounding = 0.5;
	for (d = 0; d < decimals; rounding /= 10.0, d++);
	value += rounding;

	// Integer part
	index += itoa_s((int)(value), buf+index);
	buf[index++] = '.';

	// Remove everything except the decimals
	value = value - (int)(value);

	// Convert decmial part to integer
	int ival = 1;
	for (d = 0; d < decimals; ival *= 10, d++);
	ival *= (int)value;

	// Add decimal part to string
	index += itoa_s(ival, buf+index);
	buf[index] = '\0';
  return buf;
  } 

// WARNING: capitalize source string! 
char* to_upper(char* string)
{
	char* temp;
	for (temp = string; *temp; ++temp)
		*temp = (char)upcase(*temp);
	return string;
}

char isdigit(const char ch)
{
	return (ch >= '0') && (ch <= '9');

}