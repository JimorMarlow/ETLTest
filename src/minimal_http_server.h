#pragma once
/**
 * @file minimal_http_server.h
 * @brief Ультра-лёгкий HTTP-сервер для ESP8266 (~36 байт)
 *
 * НЕТ массивов маршрутов — один dispatch callback.
 * Все буферы запросов — на СТЕКЕ в handleClient().
 */

#if defined(ESP8266)

#include <ESP8266WiFi.h>
#include <Arduino.h>

class MinimalHttpServer {
public:
    typedef void (*DispatchFn)(MinimalHttpServer&, const char* uri, bool is_post, const char* body, size_t body_len);

    MinimalHttpServer(uint16_t port = 80) : m_port(port), m_server(port) {}
    void begin() { m_server.begin(); }
    void stop()  { m_server.close(); }

    void onDispatch(DispatchFn fn) { m_dispatch = fn; }
    void onNotFound(DispatchFn fn) { onDispatch(fn); }

    void handleClient() {
        WiFiClient client = m_server.accept();
        if (!client) return;
        uint32_t t = millis() + 2000;
        while (!client.available() && millis() < t) { delay(1); yield(); }
        if (!client.available()) { client.stop(); return; }

        char uri[64] = {0}, body[512] = {0};
        size_t uri_len = 0, body_len = 0;
        bool post = false;

        if (parseRequest(client, uri, uri_len, body, body_len, post)) {
            m_tmp_client = &client; m_tmp_uri = uri;
            m_tmp_body = body; m_tmp_body_len = body_len; m_tmp_post = post;
            if (m_dispatch) m_dispatch(*this, uri, post, body, body_len);
            else send(404, "text/plain", "Not Found");
        }
        client.stop(); m_tmp_client = nullptr;
    }

    WiFiClient& client() { return *m_tmp_client; }
    bool hasArg(const char* n) const { return n && strcmp(n,"plain")==0 && m_tmp_body_len>0; }
    String arg(const char* n) const { return (n&&strcmp(n,"plain")==0)?String(m_tmp_body):String(); }
    const String& method() const { static const String G="GET",P="POST"; return m_tmp_post?P:G; }
    String uri() const { return m_tmp_uri?String(m_tmp_uri):String(); }
    void sendHeader(const String&,const String&){}
    void send_P(int code,const char* ct,PGM_P body){reply_pgm(code,ct,body);}
    void send(int code,const char* ct,const char* body){reply_int(code,ct,body,strlen(body));}
    void send(int code,const char* ct,const String& body){reply_int(code,ct,body.c_str(),body.length());}
    // alias для совместимости
    void reply(int code,const char* ct,const char* body){send(code,ct,body);}

private:
    uint16_t m_port;
    WiFiServer m_server;
    DispatchFn m_dispatch=nullptr;
    WiFiClient* m_tmp_client=nullptr;
    const char* m_tmp_uri=nullptr;
    const char* m_tmp_body=nullptr;
    size_t m_tmp_body_len=0;
    bool m_tmp_post=false;

    void reply_int(int code,const char* ct,const char* body,size_t len){
        const char* st=code==200?"OK":code==400?"Bad Request":"Not Found";
        char h[128];
        snprintf_P(h,sizeof(h),PSTR("HTTP/1.1 %d %s\r\nContent-Type: %s\r\nContent-Length: %u\r\nConnection: close\r\n\r\n"),code,st,ct,(unsigned)len);
        m_tmp_client->print(h);
        m_tmp_client->flush();
        // Отправка мелкими чанками + delay — TCP стек успевает отправлять
        size_t sent=0;
        while(sent<len){
            size_t n=min((size_t)128,len-sent);
            m_tmp_client->write(body+sent,n);
            m_tmp_client->flush();
            sent+=n;
            delay(1);
            yield();
        }
    }
    void reply_pgm(int code,const char* ct,PGM_P body){
        const char* st=code==200?"OK":"Not Found";size_t blen=strlen_P(body);
        char h[128];
        snprintf_P(h,sizeof(h),PSTR("HTTP/1.1 %d %s\r\nContent-Type: %s\r\nContent-Length: %u\r\nConnection: close\r\n\r\n"),code,st,ct,(unsigned)blen);
        m_tmp_client->print(h);
        m_tmp_client->flush();
        // write_P читает напрямую из PROGMEM — НЕ аллоцирует RAM для источника
        size_t s=0;
        while(s<blen){size_t n=min((size_t)128,blen-s);m_tmp_client->write_P(body+s,n);m_tmp_client->flush();s+=n;delay(1);yield();}
    }

    static bool parseRequest(WiFiClient& c,char* uri,size_t& ulen,char* body,size_t& blen,bool& post){
        int ch;while((ch=c.read())>0&&ch<=' ');if(ch<0)return false;
        char m[8];int mp=0;while(ch>0&&ch!=' '&&mp<7)m[mp++]=(char)ch,ch=c.read();m[mp]=0;
        if(ch!=' ')return false;
        if(strcmp(m,"POST")==0)post=true;else if(strcmp(m,"GET")!=0)return false;
        ulen=0;while((ch=c.read())>0&&ch!=' '&&ulen<63)uri[ulen++]=(char)ch;uri[ulen]=0;
        while(ch>0&&ch!='\n')ch=c.read();
        char line[80];int lp=0;size_t clen=0;
        while((ch=c.read())>0){
            if(ch=='\n'){line[lp]=0;if(lp==0)break;if(!strncmp(line,"Content-Length:",15))clen=atol(line+15);lp=0;}
            else if(ch!='\r'){if(lp<79)line[lp++]=(char)ch;}
        }
        if(post&&clen>0){size_t rd=min(clen,(size_t)511);blen=0;
            while(blen<rd&&c.available()){int b=c.read();if(b<0)break;body[blen++]=(char)b;}body[blen]=0;
            while(clen>blen&&c.available())c.read();}
        return ulen>0;
    }
};

#endif // ESP8266
