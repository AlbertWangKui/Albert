 /***************************************************************************
  *                                                                         *
  *  Copyright 2019- Inventec.  All rights reserved.                        *
  *                                                                         *
  ***************************************************************************
  *
  *           Name:  iecCliHistory.c
  *          Title:  IEC Arbok Source File
  *     Programmer:  Albert Wang
  *  Creation Date:  Feb 25, 2019
  *
  *  Version History
  *  ---------------
  *
  *  Date      Who   Description
  *  --------  ---   -------------------------------------------------------
  *  25/02/19  AW    Initial version.
  *
  *
  * Description
  * ------------
  *  This file is the  source file for Clicore.c.
  *
  *-------------------------------------------------------------------------
  */
 /*
 ** Include Files
 */

#include "iec.h"

 
/**
 * @Name:   void cliGetSearchString(FILE *PtrOutFileHandle)
 *
 * @Description: This function will read string from history heap. and then check
 *               return corresonding value from last input string.
 *
 * @param  RepeatTime  show times of up_key or down_key, real value is disparity 
 *         maxLength: the max of 
 *         ptrCmdList: point to command list.
 *
 * @return last string of history
 * @Note   if get string is "reset", it need to free corresonding heap.
 *
 *****************************************************************************/
char* cliSearchString(U8 RepeatTime, U8 maxLength, PTR_CLI_CMD_HIS_NODE ptrCmdList, FILE* PtrOutFileHandle)
{   
    int i = 1;
    PTR_CLI_CMD_HIS_NODE head = NULL;
    U8 cur = 0;

    if(ptrCmdList == NULL)
    {   fputs("hahah\r\n", PtrOutFileHandle);
        return NULL;
    }
    head = ptrCmdList;
    for( ;ptrCmdList; ptrCmdList = ptrCmdList->PtrNext, i++)
    {  
        if(i == RepeatTime)
        {     
              cur = strlen((char*)ptrCmdList->Command);
              if((cur-maxLength) > 0)
              {
                 cliEchoSpace(cur - maxLength, PtrOutFileHandle);
              }else
              {
                 cliEchoSpace(maxLength - cur, PtrOutFileHandle);
              }
              fputc('\r', PtrOutFileHandle);
              fputs(" cmd>", PtrOutFileHandle);
              fputs(ptrCmdList->Command, PtrOutFileHandle);
              break;
        }
        if(i > head->IndexCounter)
        {
           return NULL;
        }
     }
    return (char*)ptrCmdList->Command;;
}

 /**
 * @Name:   cliEchoSpace(PTR_CLI_CMD_HIS_NODE ptrCmdList, FILE* PtrOutFileHandle)
 *
 * @Description: This function will delete extra string, thereby echo normal
 *               
 * @Param parity: the  parity between max length of command and minimal length of 
 *                command.
 *        PtrOutFileHandle: point to file handle.
 * @return - void
 *
 *****************************************************************************/
 
void cliEchoSpace(U8 parity, FILE* PtrOutFileHandle)
{
     U8 copy = parity + 1;
     
      while(copy--)
       {
            fputc('\b', PtrOutFileHandle);
       }
      
       copy = parity + 1;
       while(copy--)
       {
            //override extra character.
            fputc(' ', PtrOutFileHandle);
       }
  
     return;
}
/**
* @Name:   cliInsertNode(void)
*
* @Description: This function insert command from input string. this list use 
*               tail insert, so it output according to reverse input.
*               
*
* @return - TRUE OR FALSE
* @Note     beacuse this pList is not re-malloc ,so second pointer shouln't be passed.
*****************************************************************************/
void cliInsertNode(U8 index, PU8 inputCommand, PTR_CLI_CMD_HISTORY* ppList)
{
    PTR_CLI_CMD_HIS_NODE ptrCmdNode;
    /* Allocate memory for new CLI command node */
    if( (ptrCmdNode = (PTR_CLI_CMD_HIS_NODE)malloc(sizeof(CLI_CMD_HIS_NODE))) == NULL )
        return;

    /* initialize CLI command node */
    memset(ptrCmdNode, 0 ,sizeof(CLI_CMD_HIS_NODE));
    memcpy(ptrCmdNode->Command, (char *)inputCommand, strlen((char*)inputCommand));
    ptrCmdNode->IndexCounter = index;
    /* Add new CLI command node at the head of the list */
    ptrCmdNode->PtrNext = (*ppList)->PtrCliCmdListHead;
    (*ppList)->PtrCliCmdListHead = ptrCmdNode;

    /* Increment CLI command count */
    (*ppList)->CliCommandCount++;
    //cliPrintCmd(*ppList);
}
/**
* @Name:   void cliFreeList(void)
*
* @Description: This function free momery of history command.
*               
* @return -  void 
*
*****************************************************************************/
void cliFreeList(PTR_CLI_CMD_HISTORY pCmdList)
{
       PTR_CLI_CMD_HIS_NODE head = NULL;
       PTR_CLI_CMD_HIS_NODE p = NULL;

       if(pCmdList == NULL)
          return;
       head = pCmdList->PtrCliCmdListHead->PtrNext;
       p = NULL;
       while(head)
       {
           p = head;
           head = head->PtrNext;
           free(p);
       }
       printf("free all of history!");
       return;
}

/**
* @Name:   void cliPrintCmd(PTR_CLI_CMD_HISTORY ptrCmdList)
*
* @Description: This function only just test a list if is  normal.
*               
* @return -  void 
*
*****************************************************************************/

void cliPrintCmd(PTR_CLI_CMD_HISTORY ptrCmdList)
{

    PTR_CLI_CMD_HIS_NODE ptrCliCmdNode = ptrCmdList->PtrCliCmdListHead;
    
    for( ;ptrCliCmdNode;ptrCliCmdNode = ptrCliCmdNode->PtrNext)
    {

       printf("\r\n%d-- :%s\r\n", ptrCliCmdNode->IndexCounter, ptrCliCmdNode->Command);

    }
}


