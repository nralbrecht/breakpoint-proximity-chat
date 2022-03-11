struct ClientStateUpdate {
    unsigned char version;
    char uuid[32];
    float x;
    float y;
    float z;
    bool isUsingRadio;
};

struct ServerStateReportHeader {
    unsigned char version;
    unsigned int userCount;
};
