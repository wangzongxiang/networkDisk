#include "protocol.h"

PDU* mkPDU(unsigned int msglen){
    unsigned int len=sizeof(PDU)+msglen;
    PDU* pdu=(PDU*)malloc(len);
    memset(pdu,0,len);
    pdu->msglen=msglen;
    pdu->PDUlen=len;
    return pdu;
}
