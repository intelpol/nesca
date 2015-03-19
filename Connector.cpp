#include <Connector.h>

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
int _pingMyTarget(char *ip)
{
    HANDLE hIcmpFile;
    unsigned long ipaddr = INADDR_NONE;
    DWORD dwRetVal = 0;
    char SendData[32] = "Data Buffer";
    LPVOID ReplyBuffer = NULL;
    DWORD ReplySize = 0;

    ipaddr = inet_addr(ip);

    if (ipaddr == INADDR_NONE)
    {
        stt->doEmitionRedFoundData("[Pinger] INADDR_NONE! [" + QString(ip) + "]");
        return 0;
    }

    hIcmpFile = IcmpCreateFile();
    if (hIcmpFile == INVALID_HANDLE_VALUE)
    {
        stt->doEmitionRedFoundData("[Pinger] Unable to open handle. [" + QString::number(GetLastError()) + "]");
        return 0;
   }

    ReplySize = sizeof(ICMP_ECHO_REPLY) + sizeof(SendData);
    ReplyBuffer = (VOID*) malloc(ReplySize);
    if (ReplyBuffer == NULL)
    {
        stt->doEmitionRedFoundData("[Pinger] Unable to allocate memory.");
        return 0;
    }

    dwRetVal = IcmpSendEcho(hIcmpFile, ipaddr, SendData, sizeof(SendData),
        NULL, ReplyBuffer, ReplySize, gPingTimeout*1000);
    if (dwRetVal != 0) {
        PICMP_ECHO_REPLY pEchoReply = (PICMP_ECHO_REPLY)ReplyBuffer;
        struct in_addr ReplyAddr;
        ReplyAddr.S_un.S_addr = pEchoReply->Address;
        printf("\tSent icmp message to %s\n", "127.0.0.1");
        if (dwRetVal > 1)
        {
            if(gDebugMode) stt->doEmitionYellowFoundData("[Pinger] Received " + QString::number(dwRetVal) + " icmp message responses.");
        }
        else
        {
            if(gDebugMode) stt->doEmitionYellowFoundData("[Pinger] Received " + QString::number(dwRetVal) + " icmp message responses.");
        }

        if(gDebugMode) stt->doEmitionYellowFoundData("[Pinger] Received from: " + QString(inet_ntoa( ReplyAddr )) + "; Status = " + QString::number(pEchoReply->Status) + "; Roundtrip time = " + QString::number(pEchoReply->RoundTripTime) + "ms.");
        return 1;
    }
    else
    {
        printf("\tCall to IcmpSendEcho failed.\n");
        printf("\tIcmpSendEcho returned error: %ld\n", GetLastError() );
        if(gDebugMode) stt->doEmitionRedFoundData("[Pinger] Call to IcmpSendEcho failed. IcmpSendEcho returned error: " + QString::number(GetLastError()));
        return 0;
    };
}
#else
int _pingMyTarget(char *ip)
{
    FILE *pipe = popen(("ping -w " + std::to_string(gPingTimeout) + " " + ip).c_str(), "r");
    if(!pipe) {
        stt->doEmitionRedFoundData("Ping pipe failed: cannot open pipe.");
        perror("pipe");
        return 0;
    }

    char buffer[128] = {0};
    std::string result;

    while(!feof(pipe)) {
        if(fgets(buffer, 128, pipe) != NULL){
            result += buffer;
        }
    }
    pclose(pipe);

    if(strstr((char*)result.c_str(), "100% packet loss") != NULL) return 0;
    return 1;
}
#endif

int _sshConnect(char *user, char *pass, char *host, int port)
{
    char hostStr[128] = {0};
    ZeroMemory(hostStr, sizeof(hostStr));
    strcpy(hostStr, user);
    strcat(hostStr, "@");
    strcat(hostStr, host);

    ssh_session my_ssh_session = ssh_new();
    if (my_ssh_session == NULL)
    {
        ssh_free(my_ssh_session);
        return -1;
    };

    ssh_options_set(my_ssh_session, SSH_OPTIONS_HOST, hostStr);
    ssh_options_set(my_ssh_session, SSH_OPTIONS_PORT, &port);
    //ssh_options_set(my_ssh_session, SSH_OPTIONS_LOG_VERBOSITY, &verbosity);
    //ssh_options_set(my_ssh_session, SSH_OPTIONS_LOG_VERBOSITY_STR, &verbosity);
    //ssh_options_set(my_ssh_session, SSH_OPTIONS_STRICTHOSTKEYCHECK, 0);
    int sshTimeout = gTimeOut + 1;
    ssh_options_set(my_ssh_session, SSH_OPTIONS_TIMEOUT, &sshTimeout);

    int rc = ssh_connect(my_ssh_session);

    if (rc != SSH_OK)
    {
        ssh_disconnect(my_ssh_session);
        ssh_free(my_ssh_session);
        ++offlines;
        return -2;
    }
    else
    {
        rc = ssh_userauth_password(my_ssh_session, NULL, pass);
        if (rc != SSH_AUTH_SUCCESS)
        {
            ssh_disconnect(my_ssh_session);
            ssh_free(my_ssh_session);
            return -1;
        };
    };
    ssh_disconnect(my_ssh_session);
    ssh_free(my_ssh_session);
    ++ssh;
    return 0;
}

char _get_ssh_banner(char *ip, int port)
{
    char recvBuff[256] = {0};
    std::string buffer;
    Connector::nConnect(ip, port, &buffer);

    int sz = buffer.size();

    if(sz != 0)
    {
        strncpy(recvBuff, buffer.c_str(), sz < 256 ? sz : 256);
    };

    return *recvBuff;
}

int check_ssh_pass(char *user, char *pass, char *userPass, char *host, int port, std::string *buffer, const char *banner)
{
    int res = -1;
    if(BALogSwitched) stt->doEmitionBAData("Probing SSH: " + QString(user) + ":" + QString(pass) + "@" + QString(host) + ":" + QString::number(port));
    res = _sshConnect(user, pass, host, port);
    if(res == 0)
    {
        stt->doEmition_BAGreenData("[+] SSH: " + QString(user) + ":" + QString(pass) + "@" + QString(host));
        buffer->append(userPass);
        buffer->append("@");
        buffer->append(host);
        buffer->append("|+|");
        buffer->append(banner);
        return 0;
    };
    return res;
}

int _EstablishSSHConnection(char *host, int port, std::string *buffer, const char *banner)
{
    char login[32] = {0};
    char pass[32] = {0};
    char temp[64] = {0};
    BruteUtils::BConInc();
    int sz = 0;
    char *ptr1 = 0;
    int res = -1;

    for(int i = 0; i < MaxSSHPass; ++i)
    {
        if(globalScanFlag == false) break;
        strcpy(temp, sshlpLst[i]);
        ptr1 = strstr(temp, ":");
        sz = ptr1 - temp;
        strncpy(login, temp, sz);
        strcpy(pass, ptr1 + 1);
        res = check_ssh_pass(login, pass, temp, host, port, buffer, banner);
        ZeroMemory(login, sizeof(login));
        ZeroMemory(pass, sizeof(pass));
        ZeroMemory(temp, sizeof(temp));

        if(res == 0)
        {
            if(i == 0) return -2; //Failhit
            BruteUtils::BConDec();
            return 1;
        }
        else if(res == -2)
        {
            BruteUtils::BConDec();
            return -2;
        };

        Sleep(500);
    };
    BruteUtils::BConDec();
    return -1;
}

QString strIP;
QString strPort;
int Connector::_SSHLobby(char *ip, int port, std::string *buffer)
{
    const char &banner = _get_ssh_banner(ip, port);
    if(strlen(&banner) > 0)
    {
        return _EstablishSSHConnection(ip, port, buffer, &banner);
    };
    return -1;
}


struct data {
  char trace_ascii; /* 1 or 0 */
};

static
int my_trace(CURL *handle, curl_infotype type,
             char *data, size_t size,
             void *userp)
{
  if (type == CURLINFO_HEADER_OUT) {
    Activity += strlen(data);
    data[strstr(data, "\r\n\r\n") - data] = '\0';
    stt->doEmitionAddOutData(QString(data));
  }

  return 0;
}

static size_t nWriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

int Connector::nConnect(const char *ip, const int port, std::string *buffer,
                        const char *postData,
                        const std::vector<std::string> *customHeaders,
                        const std::string *lpString){
    buffer->clear();
    CURL *curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 0L);

    if (curl)
    {
        if (MapWidgetOpened) {
            struct data config;
            config.trace_ascii = 1; /* enable ascii tracing */
            curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, my_trace);
            curl_easy_setopt(curl, CURLOPT_DEBUGDATA, &config);
            curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
        }
        curl_easy_setopt(curl, CURLOPT_URL, ip);
        curl_easy_setopt(curl, CURLOPT_PORT, port);
        curl_easy_setopt(curl, CURLOPT_USERAGENT,
                         "Mozilla/5.0 (X11; Linux x86_64; rv:35.0) Gecko/20100101 Firefox/35.0");
        curl_easy_setopt(curl, CURLOPT_HEADER, 1L);
        curl_easy_setopt(curl, CURLOPT_AUTOREFERER, 1L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, nWriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, buffer);
        int proxyPort = std::atoi(gProxyPort);
        if(strlen(gProxyIP) != 0 && (proxyPort > 0 && proxyPort < 65535)) {
            curl_easy_setopt(curl, CURLOPT_PROXY, gProxyIP);
            curl_easy_setopt(curl, CURLOPT_PROXYPORT, proxyPort);
        } else {
            curl_easy_setopt(curl, CURLOPT_PROXY, "");
        }
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, gTimeOut);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, gTimeOut);

        if(postData != NULL) {

            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postData);
        };

        if(customHeaders != NULL) {

            struct curl_slist *chunk = NULL;

            for(auto &ch : *customHeaders) {

                chunk = curl_slist_append(chunk, ch.c_str());
            }

            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);
        }

        if(lpString != NULL) {
            curl_easy_setopt(curl, CURLOPT_HTTPAUTH, (long)CURLAUTH_ANY);
            //curl_easy_setopt(curl, CURLOPT_FTPLISTONLY, TRUE);
            curl_easy_setopt(curl, CURLOPT_USERPWD, lpString->c_str());
        };

        if(curl_easy_perform(curl) == CURLE_OK) {
            curl_easy_cleanup(curl);
        } else {
            curl_easy_cleanup(curl);
            return -1;
        }

    } else {
        stt->doEmitionRedFoundData("Curl error.");
        return -1;
    };

    if(MapWidgetOpened) stt->doEmitionAddIncData(QString(ip), QString(buffer->c_str()));

    Activity += buffer->size();
    return buffer->size();
}

int Connector::_ConnectToPort(char *ip, int port, char *hl)
{
    if(gPingNScan)
    {
        if(_pingMyTarget(ip) == 0)
        {
            return -2;
        };
    };

    std::string buffer;
    int size = 0;

    if(port == 22) size = _SSHLobby(ip, port, &buffer);
    else size = nConnect(ip, port, &buffer);

//    if(size > 0)
//    {
//        ++Alive;
//        ++found;
//        stt->doEmitionChangeParsed(QString::number(saved) + "/" + QString::number(found));
//        Lexems lx;
//        lx._filler(port, buffer.c_str(), ip, size, &lx, hl);
//    };

    return 0;
}