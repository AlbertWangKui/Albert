/***************************************************************************
 *                                                                         *
 *  Copyright 2018- Inventec.  All rights reserved.                        *
 *                                                                         *
 ***************************************************************************
 *
 *           Name:  iecCliHistory.h
 *          Title:  iec CLI Header File
 *     Programmer:  Albert Wang
 *  Creation Date:  Feb 25, 2019
 *
 *  Version History
 *  ---------------
 *
 *  Date      Who   Description
 *  --------  ---   -------------------------------------------------------
 *  07/09/18  AW    Initial version.
 *
 *
 * Description
 * ------------
 *  This file is the header file for cli history module.
 *  it contains create, insert, search functions.
 *
 *-------------------------------------------------------------------------
 */
#ifndef _IEC_CLI_HISTORY_H
#define _IEC_CLI_HISTORY_H
/*
** Include Files
*/

/*
** Preprocessor Constants
*/


/*
** Macros
*/

/*
** Enumerated types
*/
typedef struct _CMD_HIS_NODE CLI_CMD_HIS_NODE, *PTR_CLI_CMD_HIS_NODE, **PTR_PTR_CLI_CMD_HIS_NODE;

struct _CMD_HIS_NODE
{
    /* Pointer to command string.*/
    char  Command[18];

    S32  IndexCounter;

    /* Pointer to next node in the list*/
    PTR_CLI_CMD_HIS_NODE    PtrNext;
} __attribute__((packed));


/*
** Typedefs
*/
typedef  struct _CLI_CMD_HISTORY   CLI_CMD_HISTORY, *PTR_CLI_CMD_HISTORY;


struct _CLI_CMD_HISTORY
{
    /* No. of record commands */
    U16 CliCommandCount;
    /* Command list head pointer */
    PTR_CLI_CMD_HIS_NODE PtrCliCmdListHead;
} ;

/*
** Variables
*/

/*
** Function Prototypes
*/
void cliPrintCmd(PTR_CLI_CMD_HISTORY ptrCmdList);


char* cliSearchString(U8 RepeatTime,  U8 maxLength, PTR_CLI_CMD_HIS_NODE ptrCmdList, FILE* PtrOutFileHandle);


void cliEchoSpace(U8 parity, FILE* PtrOutFileHandle);

void cliInsertNode(U8 index, PU8 inputCommand, PTR_CLI_CMD_HISTORY* ppList);

void cliFreeList(PTR_CLI_CMD_HISTORY pCmdList);


#endif

