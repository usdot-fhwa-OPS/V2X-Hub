

#include "base64.h"

using namespace std; 

string hex2bin(char c)
{
	switch(toupper(c))
    {
        case '0': return "0000";
        case '1': return "0001";
        case '2': return "0010";
        case '3': return "0011";
        case '4': return "0100";
        case '5': return "0101";
        case '6': return "0110";
        case '7': return "0111";
        case '8': return "1000";
        case '9': return "1001";
        case 'A': return "1010";
        case 'B': return "1011";
        case 'C': return "1100";
        case 'D': return "1101";
        case 'E': return "1110";
        default : return "1111";
    }
}

char bin2hex(string b)
{
	const char *c=b.c_str(); 
	int dec = strtol(c,nullptr,2);

	if (dec >=0 && dec <=9)
		return dec+'0'; 
	else
		return dec+'A'-10;  
}

char bin2base64(string s)
{


	const char *c= s.c_str(); 

	int dec = strtol(c,nullptr,2); 

	if ( dec >= 0 && dec <= 25 )
		return 'A'+dec;
	else if ( dec >= 26 && dec <= 51 )
		return 'a'+dec-26; 
	else if ( dec >= 52 && dec <= 61 )
		return '0'+dec-52; 
	else if (dec == 62)
		return '+';
	else 
		return '/';

	return '-'; // this would be error but is a failsafe check, shouldnt happen.

}

string dec2bin(int a)
{
	string out="000000";
	int i=0; 
	while(a)
	{
		out[out.length()-1-i]=char('0'+(a%2)); 
		a=a/2; 
		i++; 
	}
	return out; 

}

string base642bin(char b64)
{
	if (b64 >='A' && b64 <='Z')
		 return dec2bin(b64-'A'); 
	else if (b64 >='a' && b64 <='z')
		return dec2bin(b64-'a'+26); 
	else if (b64 >='0' && b64 <='9') 
		return dec2bin(b64-'0'+52); 
	else if (b64 == '+')
		return dec2bin(62);
	else 
		return dec2bin(63);
}


void  hex2base64(string hexstr, string& base64str)
{

	// convert hex string to binary, 8 bits 
	// take 6 bits chops to convert to base64

	string hexbin=""; 
	int i=0; 

	while(i<hexstr.length())
		hexbin+=hex2bin(hexstr[i++]);
	
	int padcount= (int)(24 - (int)(hexbin.length()%24))/6;

	i=0; 
	while(i<hexbin.length())
	{
		string s = hexbin.substr(i,6); 

		if(s.length()<6)
		{
			for(int j=0;j<6-s.length();j++)
			s+="0";
		}

		base64str+=bin2base64(s);

		i+=6; 
	} 

	for (i=0;i<padcount;i++)
	base64str+='='; 

}



void base642hex(string base64str, string& hexstr)
{ // this function decodes base64 stream and returns a hex stream 


	int i =0;
	string binstr=""; 
	string padchar = "=";
	int padcount=0; 

	while (i++<base64str.length())
	{	
		if(base64str[i-1]=='=')
			continue; 
		else 
			binstr+=base642bin(base64str[i-1]);
	}

	size_t padindex = base64str.find(padchar); 
	if (padindex != string::npos)
		padcount=base64str.length()-padindex; 
	
	binstr=binstr.substr(0,binstr.length()-padcount*2); //remove twice the number of padcount bits from the end 
	i=0;
	while(i<binstr.length())
	{
		hexstr+=bin2hex(binstr.substr(i,4)); //take 4 bits for hex  
		i+=4; 
	}


}
