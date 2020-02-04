#include "acr122u.h"



#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <string.h>

#include <signal.h>
typedef void (*sighandler_t)(int);
#include <sysexits.h>
#include <sys/time.h>
#include "qdebug.h"

#include <wintypes.h>
#include <winscard.h>



#define TIMEOUT 1000	/* 1 second timeout */




#define test_rv(fct, rv, hContext) \
    do { \
    if (rv != SCARD_S_SUCCESS) \
{ \
    qDebug()<<fct<<pcsc_stringify_error(rv); \
    (void)SCardReleaseContext(hContext); \
    return -1; \
    } \
    } while(0)





int CHECK(long rv)
{
    if (SCARD_S_SUCCESS != rv)
    {
        qDebug()<<pcsc_stringify_error(rv);
        return -1;
    }

    return 1;
}




ACR122U::ACR122U(QObject *parent):QObject(parent)
{
    QThread *rfidThread = new QThread;
    ACR122UWorker *rfidWatcher = new ACR122UWorker();
    rfidWatcher->moveToThread(rfidThread);
    connect(rfidThread,SIGNAL(started()),rfidWatcher,SLOT(readNFC()));
    connect(rfidWatcher,SIGNAL(getUID(QString)),this,SIGNAL(getUID(QString)));
    rfidThread->start();

}




ACR122UWorker::ACR122UWorker(QObject *parent):QObject(parent)
{

}


void ACR122UWorker::disableBuzzer(void)
{



}

int ACR122UWorker::readNFC()
{
    BYTE cmd[] = { 0xFF, 0xCA, 0x00, 0x00,0x00 };
    BYTE buzz[] = { 0xFF, 0x00, 0x52, 0x00, 0x00 };



    DWORD dwActiveProtocol, dwRecvLength;

    int current_reader;
    uint32_t rv;
    SCARDCONTEXT hContext;
    SCARD_READERSTATE *rgReaderStates_t = NULL;
    SCARD_READERSTATE rgReaderStates[1];
    DWORD dwReaders = 0, dwReadersOld;
    LPSTR mszReaders = NULL;
    char *ptr = NULL;
    const char **readers = NULL;
    int nbReaders, i;


    //initialize_terminal();

    rv = SCardEstablishContext(SCARD_SCOPE_SYSTEM, NULL, NULL, &hContext);
    test_rv("SCardEstablishContext", rv, hContext);

    rgReaderStates[0].szReader = "\\\\?PnP?\\Notification";
    rgReaderStates[0].dwCurrentState = SCARD_STATE_UNAWARE;

    rv = SCardGetStatusChange(hContext, 0, rgReaderStates, 1);
    /*  if (rgReaderStates[0].dwEventState & SCARD_STATE_UNKNOWN)
        pnp = false;*/




get_readers:
    /* free memory possibly allocated in a previous loop */
    if (NULL != readers)
    {
        free(readers);
        readers = NULL;
    }

    if (NULL != rgReaderStates_t)
    {
        free(rgReaderStates_t);
        rgReaderStates_t = NULL;
    }

    /* Retrieve the available readers list.
     *
     * 1. Call with a null buffer to get the number of bytes to allocate
     * 2. malloc the necessary storage
     * 3. call with the real allocated buffer
     */

    rv = SCardListReaders(hContext, NULL, NULL, &dwReaders);
    if (rv != SCARD_E_NO_READERS_AVAILABLE)
        test_rv("SCardListReaders", rv, hContext);

    dwReadersOld = dwReaders;

    /* if non NULL we came back so free first */
    if (mszReaders)
    {
        free(mszReaders);
        mszReaders = NULL;
    }

    mszReaders = (LPSTR)malloc(sizeof(char)*dwReaders);
    if (mszReaders == NULL)
    {
        qDebug()<< "malloc: not enough memory";
        exit(EX_OSERR);
    }

    *mszReaders = '\0';
    rv = SCardListReaders(hContext, NULL, mszReaders, &dwReaders);

    /* Extract readers from the null separated string and get the total
     * number of readers */
    nbReaders = 0;
    ptr = mszReaders;
    while (*ptr != '\0')
    {
        ptr += strlen(ptr)+1;
        nbReaders++;
    }



    if (SCARD_E_NO_READERS_AVAILABLE == rv || 0 == nbReaders)
    {

        rv = SCARD_S_SUCCESS;

        while ((SCARD_S_SUCCESS == rv) && (dwReaders == dwReadersOld))
        {
            rv = SCardListReaders(hContext, NULL, NULL, &dwReaders);
            if (SCARD_E_NO_READERS_AVAILABLE == rv)
                rv = SCARD_S_SUCCESS;
            sleep(1);

        }



        goto get_readers;
    }
    else
        test_rv("SCardListReader", rv, hContext);

    /* allocate the readers table */
    readers = (const char**)calloc(nbReaders+1, sizeof(char *));
    if (NULL == readers)
    {
        qDebug()<<"Not enough memory for readers table";
        exit(EX_OSERR);
    }

    /* fill the readers table */
    nbReaders = 0;
    ptr = mszReaders;
    while (*ptr != '\0')
    {
        readers[nbReaders] = ptr;
        ptr += strlen(ptr)+1;
        nbReaders++;






    }



    //print_readers(readers, nbReaders);

    /* allocate the ReaderStates table */
    rgReaderStates_t = (SCARD_READERSTATE*)calloc(nbReaders+1, sizeof(* rgReaderStates_t));
    if (NULL == rgReaderStates_t)
    {
        qDebug()<<"Not enough memory for readers states";
        exit(EX_OSERR);
    }

    /* Set the initial states to something we do not know
         * The loop below will include this state to the dwCurrentState
         */
    for (i=0; i<nbReaders; i++)
    {
        rgReaderStates_t[i].szReader = readers[i];
        rgReaderStates_t[i].dwCurrentState = SCARD_STATE_UNAWARE;
        rgReaderStates_t[i].cbAtr = sizeof rgReaderStates_t[i].rgbAtr;

    }



/*
    DWORD dwRecvLength2;

    BYTE pbRecvBuffer2[258];




    SCARDHANDLE hCard2;
    SCARD_IO_REQUEST pioSendPci2;



    qDebug()<<mszReaders;
    rv = SCardConnect(hContext, "ACS ACR122U PICC Interface 00 00", SCARD_SHARE_SHARED,SCARD_PROTOCOL_T0 | SCARD_PROTOCOL_T1, &hCard2, &dwActiveProtocol);
    CHECK(rv);
    rv = SCardTransmit(hCard2, &pioSendPci2, buzz, sizeof(buzz), NULL, pbRecvBuffer2, &dwRecvLength2);
    CHECK(rv);

*/

    /* If Plug and Play is supported by the PC/SC layer */






    /* Wait endlessly for all events in the list of readers
             * We only stop in case of an error
             */
    rv = SCardGetStatusChange(hContext, TIMEOUT, rgReaderStates_t, nbReaders);
    while (((rv == SCARD_S_SUCCESS) || (rv == SCARD_E_TIMEOUT)) )
    {


        /* A new reader appeared? */
        if ((SCardListReaders(hContext, NULL, NULL, &dwReaders)== SCARD_S_SUCCESS) && (dwReaders != dwReadersOld))
            goto get_readers;


        /* Now we have an event, check all the readers in the list to see what happened */

        for (current_reader=0; current_reader < nbReaders; current_reader++)
        {

            if (rgReaderStates_t[current_reader].dwCurrentState ==rgReaderStates_t[current_reader].dwEventState)
                continue;


            if (rgReaderStates_t[current_reader].dwEventState & SCARD_STATE_CHANGED)
            {
                /* If something has changed the new state is now the current  state */
                rgReaderStates_t[current_reader].dwCurrentState = rgReaderStates_t[current_reader].dwEventState;
            }
            else
                /* If nothing changed then skip to the next reader */
                continue;

            /* From here we know that the state for the current reader has
                             * changed because we did not pass through the continue statement
                             * above.


*/



            if (rgReaderStates_t[current_reader].dwEventState &
                    SCARD_STATE_IGNORE)
                qDebug()<<"Reader"<<current_reader<<rgReaderStates_t[current_reader].szReader<<":"<<"Ignore this reader";

            if (rgReaderStates_t[current_reader].dwEventState &
                    SCARD_STATE_UNKNOWN)
            {
                qDebug()<<"Reader"<<current_reader<<rgReaderStates_t[current_reader].szReader<<":"<<"Reader unknown";
                goto get_readers;
            }

            if (rgReaderStates_t[current_reader].dwEventState &
                    SCARD_STATE_UNAVAILABLE)
                qDebug()<<"Reader"<<current_reader<<rgReaderStates_t[current_reader].szReader<<":"<<"Status unavailable";

            if (rgReaderStates_t[current_reader].dwEventState &
                    SCARD_STATE_EMPTY)
            {
                // qDebug()<<"Reader"<<current_reader<<rgReaderStates_t[current_reader].szReader<<":"<<"Card removed";


            }

            if (rgReaderStates_t[current_reader].dwEventState &SCARD_STATE_PRESENT)
            {

                // qDebug()<<"Reader"<<current_reader<<rgReaderStates_t[current_reader].szReader<<":"<<"Card inserted";

                BYTE pbRecvBuffer[258];
                SCARDHANDLE hCard;
                SCARD_IO_REQUEST pioSendPci;

                rv = SCardConnect(hContext, mszReaders, SCARD_SHARE_SHARED,SCARD_PROTOCOL_T0 | SCARD_PROTOCOL_T1, &hCard, &dwActiveProtocol);
                CHECK(rv);




                switch(dwActiveProtocol)
                {
                case SCARD_PROTOCOL_T0:
                    pioSendPci = *SCARD_PCI_T0;
                    break;

                case SCARD_PROTOCOL_T1:
                    pioSendPci = *SCARD_PCI_T1;
                    break;
                }

                dwRecvLength = sizeof(pbRecvBuffer);

                rv = SCardTransmit(hCard, &pioSendPci, buzz, sizeof(buzz), NULL, pbRecvBuffer, &dwRecvLength);
                CHECK(rv);

                dwRecvLength = sizeof(pbRecvBuffer);
                rv = SCardTransmit(hCard, &pioSendPci, cmd, sizeof(cmd), NULL, pbRecvBuffer, &dwRecvLength);


                CHECK(rv);

                QString res = "";
                for(uint i=0; i<dwRecvLength; i++)
                  //res.append(QString::number(pbRecvBuffer[i],16).toUpper());
                    res.append(QString("%1").arg(pbRecvBuffer[i], 2, 16, QChar('0')).toUpper());


               qDebug()<<"UID: "<<res;

                emit getUID(res);


                rv = SCardDisconnect(hCard, SCARD_LEAVE_CARD);
                CHECK(rv);



            }


            if (rgReaderStates_t[current_reader].dwEventState &
                    SCARD_STATE_MUTE)
                qDebug()<<"Unresponsive card";

        }

        rv = SCardGetStatusChange(hContext, TIMEOUT, rgReaderStates_t,nbReaders);

    }

    /* A reader disappeared */
    if (SCARD_E_UNKNOWN_READER == rv)
        goto get_readers;

    /* If we get out the loop, GetStatusChange() was unsuccessful */
    test_rv("SCardGetStatusChange", rv, hContext);

    /* We try to leave things as clean as possible */
    rv = SCardReleaseContext(hContext);
    test_rv("SCardReleaseContext", rv, hContext);

    /* free memory possibly allocated */
    if (NULL != readers)
        free(readers);
    if (NULL != rgReaderStates_t)
        free(rgReaderStates_t);

    return EX_OK;


}

ACR122UWorker::~ACR122UWorker()
{


}


