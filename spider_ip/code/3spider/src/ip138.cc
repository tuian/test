#include"ip138.hh"
#include<iconv.h>
#include <chrono>
#include <thread>

int ip138_spider_ip::write_data(char* page_data,size_t size, size_t nmemb,std::string *data)
{
    if(data == NULL)
        return 0;
    data->append(page_data);
    return size * nmemb;
}

ip138_spider_ip::ip138_spider_ip(std::string ip_b, std::string ip_e)
{
    step = 0;
    ip_back = 0;
    current_result = "";
    next_result = "";
    ip_begin = ip_b; 
    //ip_end做手脚
    ip_end = ip_e; 
    current_ip = ip_begin;
    once_flag = 0;

    write_current_data = "";
    write_next_data = "";
    sole = 0;
    memcpy(User_Agent,"User-Agent: Mozilla/5.0 (Windows NT 6.1; \
        WOW64; rv:52.0) Gecko/20100101 Firefox/52.0", 
            strlen("User-Agent: Mozilla/5.0 (Windows NT 6.1; \
        WOW64; rv:52.0) Gecko/20100101 Firefox/52.0"));
    memcpy(url, "www.ip138.com/ips138.asp?ip=", strlen("www.ip138.com/ips138.asp?ip=")); 
    char file_name[64] = {0};
    sprintf(file_name,"%s~~%s.txt",(char*)ip_b.data(),(char*)ip_e.data());
    std::cout<<"file_name="<<file_name<<std::endl;
    fd = open(file_name, O_RDWR|O_TRUNC|O_CREAT, 0664);
}

ip138_spider_ip::~ip138_spider_ip()
{
    std::cout<<"~ip138_spider_ip()"<<std::endl; 
}

std::string ip138_spider_ip::confirm_url(std::string ip)
{
    if(once_flag == 0)
    {
        unsigned long ip_long = inet_network(ip.data());
        char tmp[32];
        int_2_str(ip_long,tmp); 
        std::string current_ip1 = {tmp};
        std::string new_url = url + current_ip1; 
        once_flag = 1;
        return new_url;
    }
    else
    {
        if(ip != ip_end)
        {
            unsigned long ip_long = inet_network(ip.data());
            if(ip_back)
            {
                ip_long = ip_long - step + 1;
                step = 1;
                ip_back = 0;
            }
            else
            {
                ip_long += step;
            }
            char tmp[32];
            int_2_str(ip_long,tmp); 
            current_ip = {tmp};
            std::string new_url = url + current_ip; 
            return new_url;
        }
        else
        {
                std::cout<<"spider ip over"<<std::endl;
                std::string err_url = {"NULL"};
                return err_url;
        /*
            sole++;
            if(sole == 2)
            {
                std::cout<<"spider ip over"<<std::endl;
                std::string err_url = {"NULL"};
                return err_url;
            }
            else //触底返回
            {
                unsigned long ip_long1 = inet_network(ip.data()) - 256 + 1;
                step = 1;
                char tmp[32];
                int_2_str(ip_long1,tmp); 
                current_ip = {tmp};
                std::string new_url = url + current_ip; 
                return new_url;
            }
         */
        }
    }
}

void ip138_spider_ip::curl_http()
{
    struct curl_slist *slist = NULL;
    slist = curl_slist_append(slist, User_Agent);
    while(1)
    {
        std::string&& current_url = confirm_url(current_ip);
        std::cout<<"current_url="<<current_url<<std::endl;
        if(current_url == "NULL")
        {
            break;
        }
        CURL* curl = curl_easy_init(); 
        std::chrono::milliseconds sleepDuration(500);
        if (curl != NULL)
        {
            curl_easy_setopt(curl, CURLOPT_URL, current_url.data());
            curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
            curl_easy_setopt(curl, CURLOPT_HEADER, 0); 
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, slist);
            curl_easy_setopt(curl, CURLOPT_HTTPGET, 0);
            curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10);
            curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10);
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data); 
            curl_easy_setopt(curl, CURLOPT_WRITEDATA,&buffer);

            unRet = curl_easy_perform(curl);
            if (CURLE_OK == unRet)
            {
                curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &code);
                if (code >= 400)
                {
                    std::cout<<"ok, response code: "<<code<<std::endl;
                }
            }
            else
            {
                curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &code);
                std::cout<<"no, response code: "<<code<<std::endl;
            }
            curl_easy_cleanup(curl);
        }
        else
        {
            std::cout<<"curl is NULL!"<<std::endl;
        }

        parse_data();
        buffer = "";
        //sleep(1);
        std::this_thread::sleep_for(sleepDuration);
    }
}

void ip138_spider_ip::parse_data()
{
    if(buffer.size() == 0)
    {
        std::cout<<"no data!"<<std::endl;
        return;
    }
    char out[1024*1024];
    g2u((char*)buffer.data(),(size_t)buffer.length(),out,(size_t)sizeof(out));
    std::string buf(out);
    std::string detail = buf.substr(buf.find("本站数据：")+strlen("本站数据："), buf.find("<",buf.find("本站数据：")+strlen("本站数据："))-(buf.find("本站数据：")+strlen("本站数据："))); 
    //std::cout<<detail<<std::endl;

    std::string des = current_ip + "   " + detail + "\n";
    if(current_result == "" )
    {
        current_result = detail;
        write_current_data = detail;

        if(write_next_data != write_current_data)
        {
            write(fd, (char*)des.data(), strlen(des.data())); 
        }
        write_current_data = write_next_data;
        write_next_data = "";
        step = 1;

    }
    else 
    {
        next_result = detail; 
    }

    if( current_result != "" && next_result != "" )
    {
        if( !strcmp(current_result.data(),next_result.data())) 
        {
            step *= 2;
            write_next_data = detail;
            current_result = next_result;
            next_result = "";
        }
        else
        {
            current_result = ""; 
            next_result = "";
            ip_back = 1; 
        }
    }
}

void ip138_spider_ip::insert_db()
{
    //std::cout << "insert_db" << std::endl;
}

int ip138_spider_ip::code_convert(char *from_charset,char *to_charset,char *inbuf,size_t inlen,char *outbuf,size_t outlen)  
{  
    iconv_t cd;  
    char **pin = &inbuf;  
    char **pout = &outbuf;  

    cd = iconv_open(to_charset,from_charset);  
    if (cd==0)  
        return -1;  
    memset(outbuf,0,outlen);  
    if ((signed)iconv(cd,pin,&inlen,pout,&outlen) == -1)  
        return -1;  
    iconv_close(cd);  
    return 0;  
}  

int ip138_spider_ip::g2u(char *inbuf,size_t inlen,char *outbuf,size_t outlen)  
{  
    return code_convert((char*)"gb2312",(char*)"utf-8",inbuf,inlen,outbuf,outlen);  
}  

