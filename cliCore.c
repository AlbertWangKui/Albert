/***************************************************************************
 *                                                                         *
 *  Copyright 2008-2016 Avago Technologies.  All rights reserved.          *
 *                                                                         *
 ***************************************************************************
 *
 *           Name:  cliCore.c
 *          Title:  Command line interface
 *     Programmer:  Prashant Badgujar
 *  Creation Date:  March 27, 2008
 *
 *  Version History
 *  ---------------
 *
 *  Date      Who       Description
 *  --------  -------   -------------------------------------------------------
 *  07/10/18  XQJ        Added the registration of iec and arbok commands.
 *                       Added the feature to autocomplete the input command.
 *  09/28/16  EJF        SCGCQ01190364: Modified the structure "errorString"
 *                       include CLI_STATUS_INVALID_FW_IMAGE and the function
 *                       cliErrorHandler() to print it.
 *  09/23/16  RPS        SCGCQ01188635: Modified the structure "errorString"
 *                       to add new error string.
 *  07/16/15  VIM        SCGCQ00884827: Return type variable in cliGetString for
 *                       fgetc is changed from U8 to signed int S32.
 *  08/11/14  SVN        SCGCQ00738753: Copyright header updated. Updated cli
 *                       header to 'SAS3 Expander'.
 *  10/29/13  JDW        SCGCQ00447162: Updated CLI help text.
 *  08/19/13  JAS        SCGCQ00478444: Updated invalid phytest pattern error
 *                       message with new list of supported patterns.
 *  04/01/13  CDH        SCGCQ00391836: Added sas inband support to execute and
 *                       display results of SDK CLI commands.
 *  03/12/13  JAS        SCGCQ00403148: Removed space from the invalid syntax
 *                       printf() format string.
 *  03/11/13  JAS        SCGCQ00386424: Added CLI status strings and handling
 *                       for phytest command.
 *  01/28/13  DJG        Flush the output after inputting a command in
 *                       cliCommandPrompt() so that the user sees the cursor
 *                       move before command processing begins. (SCGCQ00380242)
 *  10/03/12  GF         SCGCQ00339113: Added check to avoid displaying POST
 *                       if the maximum number of telnet sessions is reached.
 *  09/18/12  JAS        SCGCQ00330021: Added invalid subsystem status return
 *  09/17/12  JAS        SCGCQ00328752: Added tftpClient status returns
 *  09/13/12  JAS        SCGCQ00331098: Updated help strings for CLI commands
 *  09/13/12  MPB        SCGCQ00333517: Fixed cliCommandPrompt() to use initial
 *                       help display function from session function pointer.
 *  08/11/12  KP         SCGCQ00319778: New return type for SCEDEBUG CLI command.
 *  08/09/12  LU         SCGCQ00317585 - remove build errors due to return status
 *  07/19/12  LU         SCGCQ000218450 Create a common error handler for error
 *                       statements.
 *  05/18/12  AKS        Information about bond option has been removed from
 *                       CLI session's header. (SCGCQ00281842)
 *  03/01/12  KAM        Update CLI header to 'LSI SAS3 x48 Expander'
 *  01/30/12  BJH        Modify the use of the FILE structure.
 *  01/10/12  DC         Re-added the call to function pointer in cliCommandPrompt()
 *                       to display help or notification at the start of a new
 *                       CLI session, as it is essential for max telnet sessions
 *                       exceeded message. For a normal or Fault CLI session,
 *                       assigned this function pointer to cliPrintFirstHelpMsg().
 *                       (SCGCQ00240804)
 *  11/03/11  AKS        Automatically display showpost when a POST error is
 *                       detected. Also, display 'help' command instead of
 *                       executing it. (SCGCQ00173102)
 *  07/30/11  DJG        Change command length logic so that you can actually
 *                       type 80 characters without a command too long error.
 *  05/30/11  DC         Added cliCreateSessionEx() and
 *                       cliMaxSessionsExceededHandler(). (SCGCQ00186440)
 *  03/28/11  DC         Added space in command error message. (SCGCQ00187115)
 *  11/19/10  DC         Modified cliCommandPrompt() to wait on FW initialization
 *                       completion. (SCGCQ00147742)
 *  10/18/10  NG         Ported SCGCQ00128287 to Cobra.
 *                       [Used macro CLI_THREAD_PREEMPT_THRESH in cliCreateSession() for
 *                        preemption threshold. ]
 *  09/29/10  NG         Fixed memset statement in cliFwFaultConsoleInit().
 *  05/17/10  TJ         Repetitive CLI operations could generate a watchdog
 *                       fault. Added a thread relinquish between CLI commands
 *                       to make the CLI more cooperative.(SCGCQ00132395)
 *  04/08/10  TJ         Moved thread priority definitions to the oemPlatform.h
 *                       header file. (SCGCQ00128287)
 *  03/24/10  DJG        Duplicated Tim's fix in cliFwFaultConsoleInit for
 *                       setting the sessionActive flag.
 *  02/17/10  ASA        Added inter module dependency checking code.
 *  12/16/09  RK         Modified cliCloseSession function to fix SCGCQ00110026.
 *                       Removed condition checking before terminating the
 *                       thread,now all threads to be terminated.
 *                       Removed checking the return value from
 *                       haliOsThreadDelete function call.
 *  10/26/09  RK         Modified to fix SCGCQ00083785.
 *                       Modified cliCreateSession to mark session as active.
 *                       Modified cliCloseSession to wait till the client thread
 *                       is not completed or terminated before deleting the
 *                       thread and releasing the resources.
 *                       Modified cliCommandPrompt to make infinite while loop
 *                       to condition based and checking for HALI_EOF as session
 *                       close identifier.
 *                       Modified cliPrintHelp to add session active condition
 *                       in loops.
 *                       Modified cliGetString to check HALI_EOF as inputstring.
 *  08/25/09  SP         Modified cliCreateSession API to clear input string and cmd param
 *                       buffers before creating new CLI session (SCGCQ00096124)
 *  08/18/09  SP         CLI re-architect. Renamed to cliCore.c This file now
 *                       contains only CLI core functionalities. Moved CLI command
 *                       and command handlers to appropriate CLI module files
 *  08/05/09  BJH        Fix CQ 89348: showtrace possible buffer overflow.
 *  07/06/09  JPS        Fix for CQ 85002: more robust handling of log string
 *                       cmd.
 *  07/01/09  NM         Fixed SCGCQ00084225. Product revision level field
 *                       corrected.
 *  06/19/09  MPB        Fixed SCGCQ00081920. Updated function prototype
 *                       cliSmpIPrintResults(). printf() calls chnaged to
 *                       fprintf() in cliSmpIPrintResults().
 *  06/18/09  sschremm   SCGCQ00083395 Fix initialization of PtrCmdParams[] in
 *                       cliParseCmd().
 *  06/17/09  MPB        SCGCQ00083331 - Changed all references of
 *                       CLI_UART_FILE_HANDLE in cli.c to STDIO_UART_IO_HANDLE.
 *  06/17/09  MPB        SCGCQ00083330 - Loop maxcount CLI_NUM_CMD chnaged to
 *                       CLI_MAX_NUM_OF_TOKENS in cliparseCmd function.
 *  06/17/09  MPB        Added cliExitSession() API for to exit TCPIP based
 *                       CLI sessions. Added compillation switch for TELNET
 *                       specific Xmodem file transfer code. Changed help text
 *                       for 'exit' command.
 *  06/16/09  SBK        SCGCQ00082738 Display SAS addr of EXP, SXP, STP ports.
 *  06/12/09  BJH        Move memory buffer for 'showtrace' to stack to allow
 *                       for execution in debug handler.
 *  06/09/09  BJH        Add tracetimestamp command.
 *  06/05/09  JPS        Added new "mirror" cmd for Port Mirroring
 *  06/05/09  sschremm   SCGCQ00081339 - Modify "thread" command.
 *  05/29/09  BJH        Add tracemask command.
 *  05/28/09  NM         SCGCQ00077637 - Added new "sasaddr" command.
 *  05/28/09  NM         SCGCQ00076740 - Added hardware revision info in
 *                       "rev" command.
 *  05/15/09  SB         Added reset option to 'counters' command to clear
 *                       the counters.
 *  05/11/09  SR         Adding the new "fan" command
 *  05/07/09  BJH        Added support for the trace commands.
 *  05/05/09  SB         Implemented 'counters' command.
 *  04/29/09  MPB        SCGCQ00069452. Updated cliGetIpCfg() to display
 *                       MAC address and DHCP enable/disable configuration
 *                       option in 'ipconfig' CLI command.
 *  04/22/09  RRK        Fixed SCGCQ00075080.
 *                       Replaced parameter PtrOutFileHandle->handle with
 *                       PtrOutFileHandle->logicalConnection in
 *                       haliTelnetServerSend call in cliTelnetGetString API.
 *  04/08/09  TJ         Added support for a fault handler CLI interface.
 *  04/07/09  BJH        Added CLI support for logging
 *  04/06/09  RRK        Fixed defect SCGCQ00069496. printf calls in
 *                       cliFileDown, cliFileUp, cliMemDown and cliMemUp replaced
 *                       with fprintf calls to redirect I/O.
 *                       Added condition to call telnetXmodemRecvData and
 *                       telnetXmodemTransmitData when commands issued on
 *                       telnet sessions.
 *                       Changed indexCounter in cliTelnetGetString function
 *                       to clear input command in case of
 *                       CLI_MAX_CMD_LINE_LENGTH exceeded.
 *  04/03/09  MPB       Fixed SCGCQ00070560. Printf calls in callStackPrint()
 *                       and haliSspTDebug() replaced with fprintf calls to
 *                       redirect I/O to proper CLI session.
 *  03/31/09  RRK        Fixed Defect SCGCQ00070321 - input passed 80 chars
 *                       Modified cliTelnetGetString to put condition to check
 *                       command input length passing max allowed length.
 *  03/18/09  PHB        Reworked on SCGCQ00068339 after validation failed.
 *  03/12/09  PHB        Resolved SCGCQ00068339 defect for mr32,mr16,mw32,mw16.
 *  03/09/09  SM         Changed the valid ISTWI range from 0-2 to 0-3.
 *  03/09/09  TMJ        Resolved merge issues from the delivery activity.
 *  03/02/09  BJH        Add fault command.
 *  03/01/09  JIM        Formatting improvements for display of Read Route Table.
 *  02/26/09  RM         Display the entry state (disable bit) of route table
 *                       entries
 *  02/16/09  MPB        Added include file tftpClient.h.
 *  02/10/09  MDJ        Added 6G user option in SSPI test cases.
 *                       Added Mode ID user option in SSPI test cases.
 *                       Changes for Zero Allocation length in SSPI test cases.
 *                       Changes in cliSspiPrintResults(), now Bytes Transferred
 *                       (BT) and Q Tag(QT) are printed in decimal fashion.
 *  02/04/09  MPB        Include file stdio.h removed as this file is included
 *                       in cli.h.
 *  02/04/09  MPB        Added ipconfig, tftpfdl and tftpful, tftpfdl and
 *                       tftpful commands.
 *  02/04/09  PHB        Making changes to resolve defect SCGCQ00062400.
 *  02/04/09  PHB        Making changes to resolve defect SCGCQ00054367.
 *  01/23/09  PHB        Added haliDoorBell() call in cliCommandPrompt API.
 *  01/22/09  TJ         Added a command to display zone permission RAM.
 *  12/31/08  JIM        Added route table read command.
 *  12/30/08  PHB        Adding Region Erase command.
 *  12/22/08  PHB        Applying code optimization suggested by Harold.
 *  12/10/08  PHB        Bug fixes for Spell Check, rdcfg command & maximum
 *                       command length.
 *  11/18/08  PHB        Added Support Read Register, MUL & MDL Commands.
 *  11/18/08  PHB        Added Support for Default Size in Memory Read Command.
 *  02/24/09  TMJ       Modified 'rev' command for clarity now that the flash
 *                      regions are not overloaded.
 *  02/18/09  SM        Changed the ISTWI channel no check from <= 2 to <= 3 to
 *                      allow the passing of 3 as channel number to iicw,iicr
 *                      and iicwr commands.
 *  11/14/08  TMJ        Changed the rev command to use the new flash table.
 *  11/11/08  MPB        Updated code to use thread input arguments using a
 *                       CLI_SESSION_INFO structure.
 *                       haliMalloc() changed to malloc().
 *  11/06/08  ADP        Added mfgConfig.h. Changed the name of API to get
 *                       default MFG pages.
 *  11/06/08  SHM        Added SSP Target Debug Print support (Call Stack).
 *  10/20/08  DSK        Added SMPI functions
 *  10/24/08  PB         Modified code MR16 and MR32 commands.
 *  10/23/08  MDJ        Added connection rate conversions in SSPI functions.
 *  10/14/08  RK         Modified to add different lookup tables
 *  10/07/08  PHB        Added API for Flash/SMPT Test & Config Page Read.
 *  08/10/08  RK         Modified to support Telnet
 *  07/08/08  MDJ        Updated SSPI demo functions
 *  07/02/08  MDJ        Added Demo Functions for SSPI
 *  07/02/08  MDJ        Added scsi3.h include file
 *  05/28/08  Prashant   Added File Up & Down load support.
 *  04/24/08  Prashant   Code Review Modifications.
 *  04/04/08  Prashant   Code Cleanup, HALOS use.
 *  03/27/08  Prashant   Initial version.
 *
 *
 * Description
 * ------------
 *  This file contains API for command line interface.
 *
 *  NOTE:   When adding calls to haliAssert or haliFault within this module,
 *          be aware that some functions within this file are called from the
 *          fault handler itself.
 *
 *          To add CLI commands to the firmware fault handling monitor, add
 *          them in the cliFwFaultConsoleCmdRegister function.
 *
 *-----------------------------------------------------------------------------
 */


#include "string.h"
#include "haliApi.h"
#include "fileStructure.h"
#include "cliCore.h"
#include "fwInit.h"
#include "cliCommon.h"
#include "cliPlatform.h"
#include "cliUplDl.h"
#include "inBandUart.h"
#include "oemPlatform.h"
#include "fwTraceDebug.h"
#include "arbokCli.h"
#include "iecCli.h"


/* Time in milliseconds for which the maximum telnet/SSH connections exceeded
 * message should be displayed before closing the connection.
 */
#define MAX_SESSION_EXCEEDED_MSG_DISPLAY_MS (2000)


/* Help command info */
static const PU8 sCmdHelp = "help";
static const PU8 sHlpHelp = "    CLI Help                help [Command]\r\n"
                            "                                - X means a mandatory argument described as x\r\n"
                            "                                - [X] means an optional argument\r\n"
                            "                                - <X> means replace argument with value\r\n"
                            "                                - X(D) means a decimal value\r\n"
                            "                                - X(H) means a hexadecimal value\r\n"
                            "                                - X(*) means special formatting is specified\r\n"
                            "                                - X(s) means a decimal value in seconds\r\n"
                            "                                - X(ms) means a decimal value in milliseconds\r\n"
                            "                                - opt1|opt2|opt3 means literally specify\r\n"
                            "                                   \"opt1\",\"opt2\", or \"opt3\"\r\n"
                            "                                - X1 X2 ... means a variable number\r\n"
                            "                                  of arguments\r\n\n";
static const PU8 sPtrHelpHeader = "    Test                    Command";

static const PU8 sPtrCliCmdNotFound = " \r\n\nInvalid Command. Use help command for CLI"
                                        " Command Help.\r\n";

/* This object holds all common CLI commands registered with CLI core */
static CLI_CMD_LIST sCliCmdList;

/*This object holds all history command valid by input correct command.*/
static CLI_CMD_HISTORY sCliCmdHistory;

/* These structures hold the session information and the command table for the
 * fault handler CLI interface.
 */
static CLI_SESSION_INFO     sCliFwFaultSessionInfo;
/* Flag to avoid multiple initialization of CLI Core */
static BOOL sCliCoreInitialized = FALSE;

static const char *errorString[] =
{
    {""},                                                       //CLI_STATUS_SUCCESS
    {"Invalid Command Syntax!"},                                //CLI_STATUS_INVALID_SYNTAX
    {"Invalid Write Type!"},                                    //CLI_STATUS_INVALID_WRT_TYPE
    {"Invalid Address!"},                                       //CLI_STATUS_INVALID_ADDRESS
    {"Invalid High SAS Address!"},                              //CLI_STATUS_INVALID_HIGH_SAS_ADD
    {"Invalid Low SAS Address!"},                               //CLI_STATUS_INVALID_LOW_SAS_ADD
    {"Valid options are 'on' and 'off'"},                       //CLI_STATUS_INVALID_OPTION
    {"Invalid Parameter!"},                                     //CLI_STATUS_INVALID_PARAMETER
    {"Invalid Memory Length!"},                                 //CLI_STATUS_INVALID_MEM_LEN
    {"Invalid Allocation Length!"},                             //CLI_STATUS_INVALID_ALL_LEN
    {"Data Overflow. Valid Range is 00 to FF."},                //CLI_STATUS_DATA_OVERFLOW_8
    {"Data Overflow. Valid Range is 0000 to FFFF."},            //CLI_STATUS_DATA_OVERFLOW_16
    {"Data Overflow. Valid Range is 00000000 to FFFFFFFF."},    //CLI_STATUS_DATA_OVERFLOW_32
    {"Invalid Write data!"},                                    //CLI_STATUS_INVALID_WRT_DATA
    {"Failed - Thread Get Info Error."},                        //CLI_STATUS_THREAD_ERROR
    {"Failed to read configuration page."},                     //CLI_STATUS_READ_CFG_ERR
    {"Invalid Page Id."},                                       //CLI_STATUS_INVALID_PAGE_ID
    {"Invalid Region Number."},                                 //CLI_STATUS_INVALID_REGION
    {"Erase flash region failed"},                              //CLI_STATUS_ERASE_FAILED
    {"Invalid Erase Parameter!"},                               //CLI_STATUS_INVALID_ERASE_PARAM
    {"Failed to read Mfg config Version page 0xFF0F."},         //CLI_STATUS_RD_MFG_FAILED
    {"Error reading log."},                                     //CLI_STATUS_RD_LOG_ERROR
    {"Invalid String length."},                                 //CLI_STATUS_INVALID_STRING
    {"Error quotes around string not found."},                  //CLI_STATUS_NO_QUOTES
    {"Error adding log."},                                      //CLI_STATUS_LOG_ADD_ERR
    {"Error clearing log."},                                    //CLI_STATUS_CLR_LOG_ERR
    {"Error reading trace."},                                   //CLI_STATUS_TRACE_ERR
    {"Error clearing trace."},                                  //CLI_STATUS_CLEAR_TRACE_ERR
    {"Please provide a hex number."},                           //CLI_STATUS_ENTER_HEX
    {"Invalid severity!  Valid severity range is (0-3)."},      //CLI_STATUS_TRACE_VALID_RANGE
    {"Failed to write config page."},                           //CLI_STATUS_WRT_FAILED
    {"Invalid Tracemask."},                                     //CLI_STATUS_INVALID_TRCMASK
    {"Invalid Subsystem."},                                     //CLI_STATUS_INVALID_SUBSYSTEM
    {"Invalid Byte Number!"},                                   //CLI_STATUS_BYTE_ERR
    {"Invalid Channel!"},                                       //CLI_STATUS_INVALID_CHANNEL
    {"Invalid source phy number!"},                             //CLI_STATUS_INVALID_SRC_PHY
    {"Invalid receive destination phy number."},                //CLI_STATUS_INVALID_RX_PHY
    {"Invalid transmit destination phy number."},               //CLI_STATUS_INVALID_TX_PHY
    {"mirror enable/disable failed"},                           //CLI_STATUS_MIR_EN_DISEN_FAIL
    {"Invalid mirror phy."},                                    //CLI_STATUS_INVALID_MIR_PHY
    {"Insufficient Memory, can not continue"},                  //CLI_STATUS_INSUFF_MEM
    {"Ethernet compliance test command only on UART."},         //CLI_STATUS_UART_ONLY
    {"Ethernet compliance test already disabled"},              //CLI_STATUS_ETC_DIABLED
    {"Failed to disable Ethernet compliance test."},            //CLI_STATUS_ETC_DISABLE_FAIL
    {"Incorrect Timeout Value"},                                //CLI_STATUS_INVALID_TIMEOUT
    {"Invalid Mode Value"},                                     //CLI_STATUS_INVALID_MODE
    {"Invalid Interface"},                                      //CLI_STATUS_INVALID_INTERFACE
    {"This expander does not have any configured SGX devices attached"},    //CLI_STATUS_NO_SGX_DEVICES
    {"Invalid Port"},                                           //CLI_STATUS_INVALID_PORT
    {"Command Length too long!"},                               //CLI_STATUS_CMD_LEN
    {"Report General failed, could not get status."},           //CLI_STATUS_RPT_GEN_FAILED
    {"Invalid Connection Rate."},                               //CLI_STATUS_INVALID_CON_RATE
    {"Xmodem Transfer Failed"},                                 //CLI_STATUS_XFER_FAILED
    {"Invalid File Length."},                                   //CLI_STATUS_INVALID_FILE_LEN
    {"Invalid Function Request."},                              //CLI_STATUS_INVALID_FUNC_REQ
    {"Invalid Buffer ID."},                                     //CLI_STATUS_INVALID_BUF_ID
    {"Error Creating Thread."},                                 //CLI_STATUS_CREATE_THRD_FAILED
    {"Error while accessing Flash details."},                   //CLI_STATUS_FLASH_ERROR
    {"Flash device not present."},                              //CLI_STATUS_NO_FLASH
    {"SourceZoneGroup must be less than or equal to 0xFF."},    //CLI_STATUS_SRC_GROUP_ERR
    {"Invalid SEEPROM data."},                                  //CLI_STATUS_INVALID_SEPRM_DATA
    {"Error reading SEEPROM."},                                 //CLI_STATUS_RD_SEPRM_ERROR
    {"Invalid Number of Parameters!"},                          //CLI_STATUS_INVALID_PARAM_NUM
    {"Illegal Argument!"},                                      //CLI_STATUS_ILLEGAL_ARG
                                                                //CLI_STATUS_ILLEGAL_RESET_ARG
    {"Illegal Argument!\r\n"\
                            "\r\nReset Usage:  reset (issues softreset)"\
                            "\r\n              reset watchdog (issues watchdog reset)"},
                                                                //CLI_STATUS_INVALID_PKT_SIZE
    {"Invalid Packet Size, "\
     "Provide packet size greater than or equal to 60 and less then or equal to 1024"},
    {"Invalid Delay, Provide delay in milliseconds"},           //CLI_STATUS_INVALID_DELAY_ARG
    {"Invalid Pattern, Provide pattern in a dword"},            //CLI_STATUS_INVALID_PATTERN_ARG
    {"Invalid parameter given to Sgpio Read Request."},         //CLI_STATUS_INVALID_PARAM_SGPIO
    {"Memory Download Failed"},                                 //CLI_STATUS_MEM_DL_FAILED
    {"Memory Upload Failed"},                                   //CLI_STATUS_MEM_UL_FAILED
    {"Firmware Download Failed"},                               //CLI_STATUS_FW_DL_FAILED
    {"Firmware Upload Failed"},                                 //CLI_STATUS_FW_UL_FAILED
    {"Invalid Buffer Offset"},                                  //CLI_STATUS_INVALID_BUF_OFFSET
    {"Invalid Memory address"},                                 //CLI_STATUS_INVALID_MEM_ADDRESS
    {"Invalid Filename Parameter"},                             //CLI_STATUS_INVALID_FILENAME
    {"Invalid IP Address"},                                     //CLI_STATUS_INVALID_IP_ADDRESS
    {"Invalid File Size"},                                      //CLI_STATUS_INVALID_FILE_SIZE
    {"Invalid Test Mode - must be int"},                        //CLI_STATUS_INVALID_PHYTEST_MODE
    {"Invalid Pattern - only 0,6,9,10,12,13 are supported"},    //CLI_STATUS_INVALID_PHYTEST_PATTERN
    {"Invalid Connection Rate - "\
     "must be 0(3G), 1(6G) or 2(12G)"},                         //CLI_STATUS_INVALID_PHYTEST_RATE
    {"Invalid Duration - must be greater than 0"},              //CLI_STATUS_INVALID_PHYTEST_DURATION
    {"Invalid Phymask"},                                        //CLI_STATUS_INVALID_PHYTEST_PHYMASK
    {"Failed to allocate memory for the command"},              //CLI_STATUS_MALLOC_FAILED
    {"Command Syntax:"},                                        //CLI_STATUS_SYNTAX
    {"Invalid Firmware Image"},                                 //CLI_STATUS_INVALID_FW_IMAGE
    {"Command Failed!"},                                        //CLI_STATUS_FAILED
    {"Info not available"},                                     //CLI_STATUS_INFO_NOT_AVAILABLE
    {""}                                                        //CLI_STATUS_ERROR_PRINTED

};

/* This list holds all cli commands not supported by the in-band cli. */
static const CLI_CMD_INFO *sPtrUnSupportedCliCmdList[] = {
                                                        /* CLI - Upload/Download */
                                                            &gCliCmdFul,
                                                            &gCliCmdFdl,
                                                            &gCliCmdMul,
                                                            &gCliCmdMdl,
                                                    #if ( ENABLE_TCPIP )
                                                            &gCliCmdTftpful,
                                                            &gCliCmdTftpfdl,
                                                            &gCliCmdTftpmul,
                                                            &gCliCmdTftpmdl,
                                                    #endif /* ENABLE_TCPIP */
                                                            /* memstat command */
                                                            &gCliCmdMemstat,
                                                        };

/* Interface function to create UART based CLI session. This is used by obsolete
 * function cliInit()
 */
extern BOOL cliUartInitSession(void);


/*
 * Function protoypes for functions that are static to this module.
 */
static CLI_STATUS cliPrintFirstHelpMsg( PTR_CLI_SESSION_INFO PtrSessionInfo );


/**
 * @Name: cliCoreInit()
 *
 * @Description: This function Initializes the CLI Core and CLI command list.
 *              CLI core should be initialized before creating any
 *              CLI sessions (UART/Telnet/SSH). This function make use of
 *              cliReleaseInit()/cliDebugInit()/oemCliInit() to register
 *              Oem/debug/release CLI commands.
 *
 *****************************************************************************/
void cliCoreInit( void )
{
   /* Perform dependency check */
    haliDepCheckModuleDependency( haliDepGetHalDependencyContext(),
                                  HALI_DEP_CLI_ID,
                                  HALI_DEP_CLI_DEPENDENCY_LIST );

    /* Make sure that CLI core is initialized ONLY ONCE */
    if( sCliCoreInitialized == FALSE )
    {
        sCliCmdList.CliCommandCount = 0;
        sCliCmdList.PtrCliCmdListHead = NULL;
        /* Register help command - Help command is registered from core itself */
        cliRegisterCommand(( PU8 )sCmdHelp, ( PU8 )sHlpHelp, cliPrintHelp, &sCliCmdList);

        /* Initialize and Register Debug/Release/OEM specific CLI commands */
        cliReleaseInit(&sCliCmdList);
        cliDebugInit(&sCliCmdList);
        oemCliInit(&sCliCmdList);

        /* Register IEC and Starmie specific CLI commands. */
        iecCliInit(&sCliCmdList);
        arbokCliInit(&sCliCmdList);
        /* Mark CLI core as initialized */
        sCliCoreInitialized = TRUE;

        /* CLI initialized */
        haliDepSetModuleInitialized( haliDepGetHalDependencyContext(),
                                     HALI_DEP_CLI_ID );
    }
}


/**
 * @Name: cliCreateSession()
 *
 * @Description: This function creates a new cli Session.
 *              Initializes CLI session object, creates the cli thread for
 *              the session and attach the cli session object with the thread
 * @param PtrCliSessionInfo     - Pointer to the CLI Session Information structure
 *
 * @param PtrCliSessionName     - String name of this CLI session
 *
 * @param FptrGetCmdString  - Pointer to function to read CLI commands for the new
 *                              CLI session. Can be NULL. if NULL, default function,
 *                              cliGetString() is used
 *
 * @return TRUE if CLI session created successfully.
 *
 * @Note This function returns FALSE if memory allocation, object creation
 *      or thread creation fails.
 *      Session specific I/O Hanlder in PtrCliSessionInfo should be set
 *      before invoking this function.
 *****************************************************************************/
BOOL cliCreateSession( PTR_CLI_SESSION_INFO PtrCliSessionInfo,
                                const U8 * PtrCliSessionName,
                                PTR_CLI_GET_COMMAND FptrGetCmdString )
{

    return cliCreateSessionEx( PtrCliSessionInfo,
                               PtrCliSessionName,
                               NULL,
                               FptrGetCmdString,
                               NULL );
}


/**
 * @Name: cliCreateSessionEx()
 *
 * @Description: This function creates a new cli Session. It is an
 *              extended version of cliCreateSession() with addional parameter
 *              to override the display help function that is called at the begining
 *              of a session from the CLI session thread.
 *              Initializes CLI session object, creates the cli thread for
 *              the session and attach the cli session object with the thread.
 *
 * @param PtrCliSessionInfo  -  Pointer to the CLI Session Information structure
 *
 * @param PtrCliSessionName  -  String name of this CLI session
 *
 * @param FptrCmdPrompt      -  Pointer to function to use for cli thread entry
 *                              function.  Can be null. If null, default
 *                              function, cliCommandPrompt(), is used.
 *
 * @param FptrGetCmdString  -   Pointer to function to read CLI commands for the new
 *                              CLI session. Can be NULL. if NULL, default function,
 *                              cliGetString() is used.
 *
 * @param FptrOverrideDisplayHelp - Function to override the default Display-Help
 *                              function that usually gets called at the start of
 *                              CLI session thread. If NULL is passed in this
 *                              parameter then the default Display-Help
 *                              function would get called else the function that
 *                              is passed here.
 *
 * @return TRUE if CLI session created successfully.
 *
 * @Note This function returns FALSE if memory allocation, object creation
 *      or thread creation fails.
 *      Session specific I/O Hanlder in PtrCliSessionInfo should be set
 *      before invoking this function.
 *****************************************************************************/
BOOL cliCreateSessionEx( PTR_CLI_SESSION_INFO PtrCliSessionInfo,
                                const U8 * PtrCliSessionName,
                                PTR_CLI_CMD_PROMPT FptrCmdPrompt,
                                PTR_CLI_GET_COMMAND FptrGetCmdString,
                                PTR_CLI_DISPLAY_HELP FptrOverrideDisplayHelp)
{
    HALI_OS_STATUS  retCliStat = HALI_OS_NO_MEMORY;
        
    PTR_CLI_CMD_PROMPT fptrCommandPrompt;

    /* Allocate space for the CLI session thread object */
    PtrCliSessionInfo->CliThreadHandle = haliOSAllocateObject(HALI_MEMORY_ID_IMEM,
                                            HALI_OS_THREAD);

    if( PtrCliSessionInfo->CliThreadHandle == HALI_OS_INVALID_HANDLE )
    {
        return FALSE;
    }
    /* Allocate space for the stack */
    if( (PtrCliSessionInfo->PtrCliThreadStack = malloc(CLI_THREAD_STACK_SIZE))
        == NULL )
    {
        /* Release thread handle object */
        haliOsReleaseObject(PtrCliSessionInfo->CliThreadHandle);
        return FALSE;
    }
    /* Initialize CLI session .*/
    if(FptrOverrideDisplayHelp != NULL)
    {
        /* Call the overriding function at the begining of CLI thread.*/
        PtrCliSessionInfo->fptrDisplyHelp = FptrOverrideDisplayHelp;
    }
    else
    {
        /* Call the default function to display help.*/
        PtrCliSessionInfo->fptrDisplyHelp = cliPrintFirstHelpMsg;
    }
    PtrCliSessionInfo->fptrGetCommandString =
        (FptrGetCmdString) ? FptrGetCmdString : cliGetString;
    PtrCliSessionInfo->TokenInCmdRcd  = 0;

     /* By default, use the command list genereated by core.
      * Can be extended, if needed, using cliRegisterCommand()
      */
    if( PtrCliSessionInfo->CliCmdList.CliCommandCount == 0 )
    {
        PtrCliSessionInfo->CliCmdList = sCliCmdList;
    }

    PtrCliSessionInfo->PtrCurCommand = NULL;
    /* clear inputString & command param buffers */
    memset( PtrCliSessionInfo->inputString, 0, sizeof(U8)*CLI_MAX_CMD_LINE_LENGTH );
    memset( PtrCliSessionInfo->PtrCmdParams, 0, sizeof(PU8)*CLI_MAX_NUM_OF_TOKENS );
    PtrCliSessionInfo->TokenInCmdRcd = 0;

    /* Mark the session as active */
    PtrCliSessionInfo->SessionActive = TRUE;


    /* Determine cli thread entry function to use when creating thread. */
    fptrCommandPrompt = (FptrCmdPrompt) ? FptrCmdPrompt : cliCommandPrompt;

    /* Create a new thread to handle this new CLI session */
    retCliStat = haliOsThreadCreate(PtrCliSessionInfo->CliThreadHandle,
                                            (U8*)PtrCliSessionName, /* CLI session thread name */
                                            fptrCommandPrompt,     /* thread function*/
                                            (U32)PtrCliSessionInfo, /* input param */
                                            PtrCliSessionInfo->PtrCliThreadStack, /* stack */
                                            CLI_THREAD_STACK_SIZE,
                                            CLI_THREAD_PRIORITY,
                                            CLI_THREAD_PREEMPT_THRESH,
                                            0,
                                            HALI_OS_AUTO_START_ENABLE);

    if( retCliStat != HALI_OS_SUCCESS )
    {
        /* Add trace containing error. */
        FW_TRACE1_MACRO(HALI_TRACE_SEVERITY_ERROR,
                        FW_TRACE_STR_CLI_THREAD_ERROR,
                        FW_TRACE_MASK_GENERAL,
                        retCliStat );

        
        cliFreeList(&sCliCmdHistory);
        
        /* Release thread handle object */
        haliOsReleaseObject(PtrCliSessionInfo->CliThreadHandle);
        /* Free stack */
        free(PtrCliSessionInfo->PtrCliThreadStack);
        return FALSE;
    }
    return TRUE;
}

/**
 * @Name:   cliInBandParseCmd()
 *
 * @Description: This function parses the command line input and
 *             calls the corresponding command handler function
 *             if parsing is done successfully.
 *
 * @param PtrString - Command line input command.
 *
 * @param PtrSessionInfo - Pointer to the CLI session information
 *                structure.
 *
 * @return - Returns status of command parsing i.e.
 *          TRUE - Command parsed successfully.
 *          FALSE - Command parsing failed.
 *
 * @note Parsing will fail if the executing command is part of
 *       the unsupported in-band command list.
 *
 *****************************************************************************/
BOOL cliInBandParseCmd( PU8 PtrString, PTR_CLI_SESSION_INFO PtrSessionInfo )
{
    /* Counters for looping, token count & token length resp.*/
    U32     loopCount, tkCount, tkLen;
    PU8     ptrToken = NULL;
    CLI_STATUS      retStatus;
    U32 counter = 0;
    U16 cmdCount = 0;

    tkCount = 0;
    tkLen = 0;

    /* Initialize the Command Param array with null string */
    for( loopCount = 0 ; loopCount < CLI_MAX_NUM_OF_TOKENS; loopCount++ )
    {
        PtrSessionInfo->PtrCmdParams[loopCount] = NULL;
    }

    /* Separate out the tokens in the command line input.*/
    ptrToken = (PU8)strtok( (char *)PtrString, CLI_CMD_DELIMS );

    while( ptrToken != NULL )
    {
        tkLen = strlen((const char*)ptrToken);

        if( tkLen >= CLI_MAX_TOKEN_LENGTH || tkCount == CLI_MAX_NUM_OF_TOKENS )
        {
            /*
             * if token length is bigger than maximum token buffer
             * then it will return error.
             */
            return FALSE;
        }

        PtrSessionInfo->PtrCmdParams[tkCount] = (PU8)ptrToken;
        tkCount++;

        ptrToken = (PU8)strtok( NULL, CLI_CMD_DELIMS );
    }

    /* Number of unsupported commands */
    cmdCount = sizeof(sPtrUnSupportedCliCmdList)/sizeof(sPtrUnSupportedCliCmdList[0]);

    /*
     * Traverse through unsupported command list to determine if this command
     * can be executed.
     */
    for( counter = 0; counter < cmdCount; counter++ )
    {
        /* If we find a positive match we will return failed status. */
        if( strncmp((char *)sPtrUnSupportedCliCmdList[counter]->PtrCmdName,
            (const char *)(PtrSessionInfo->PtrCmdParams[0]),
            CLI_MAX_TOKEN_LENGTH ) == 0 )
        {
            /*
             * Exceptions to the unsupported list are added here.
             */

            /* memstat command is allowed but only if the user does not
             * require the user to provide additional input during command
             * execution.  The memstat command requires additional input
             * whenever its spawns an additional thread to output memstat
             * info on an interval for a specific handle.  This occurs when
             * the commands token count is greater than one.
             */
            if( (strncmp( (const char *)PtrSessionInfo->PtrCmdParams[0],
                         (const char *)gCliCmdMemstat.PtrCmdName,
                         CLI_MAX_TOKEN_LENGTH) == 0 ) &&
                ( tkCount < 2 ) )
            {
                break;
            }
            else
            {
                /* Do not allow command to be executed. */
                return FALSE;
            }
        }
    }

    /*
     * Traverse through CLI command linked list to find a matching command string
     */
    for( PtrSessionInfo->PtrCurCommand = PtrSessionInfo->CliCmdList.PtrCliCmdListHead;
         PtrSessionInfo->PtrCurCommand != NULL;
         PtrSessionInfo->PtrCurCommand = PtrSessionInfo->PtrCurCommand->PtrNext )
    {
        if( strncmp((char *)(PtrSessionInfo->PtrCurCommand->Command),
                    (const char *)PtrSessionInfo->PtrCmdParams[0],
                    CLI_MAX_TOKEN_LENGTH) == 0 )
        {
            PtrSessionInfo->TokenInCmdRcd = tkCount;

            /* Check whether Pointer to function is NULL. */
            if( PtrSessionInfo->PtrCurCommand->PtrToFunCall != NULL )
            {
                /* Call Corresponding callback function. */
                retStatus = PtrSessionInfo->PtrCurCommand->PtrToFunCall(PtrSessionInfo);
                cliErrorHandler( retStatus, PtrSessionInfo );
            }

            break;
        }
    }
    /*
     * Parser status. True - Command found in command database,
     * False - command not found.
     */
    return (PtrSessionInfo->PtrCurCommand != NULL);
}

/**
 * @Name:   cliInBandCommandPrompt()
 *
 * @Description: This function reads command using session specific interface function
 *          to wait for input string to be copied into command
 *          buffer and pass the string to cli command string
 *          parser (cliInBandParseCmd())
 *
 * @param  ThreadInput - CLI Thread entry function input parameter
 *          Holds a pointer of type PTR_CLI_SESSION_INFO. Contains CLI session
 *          specific information
 *
 *****************************************************************************/
void cliInBandCommandPrompt( U32 ThreadInput )
{
    PTR_CLI_SESSION_INFO ptrSessionInfo = NULL;
    HALI_OS_STATUS osStatus;

    ptrSessionInfo = (PTR_CLI_SESSION_INFO)ThreadInput;

    /* Block till FW initialization completes.*/
    osStatus  = haliDepSyncWait( HALI_DEP_SYNC_FW_INIT_COMPLETE_EVT,
                                  HALI_OS_WAIT_FOREVER );

    haliAssert( HALI_OS_API_SUCCESS(osStatus), FAULT_HAL_DEP_WAIT_FAILURE );

    /* While session is active */
    while( ptrSessionInfo->SessionActive == TRUE )
    {
        osStatus = inBandUartReadCommandBuffer( ptrSessionInfo->inputString );
        if( osStatus == HALI_OS_SUCCESS )
        {
            /* check if command is not empty or CR or EOF then process it. */
            /* EOF is treated as end of CLI session and we don't want to process it.*/
            if( (ptrSessionInfo->inputString[0] != 0x00) &&
                (ptrSessionInfo->inputString[0] != CLI_CR) &&
                (ptrSessionInfo->inputString[0] != HALI_EOF) )
            {
                if( cliInBandParseCmd(ptrSessionInfo->inputString, ptrSessionInfo)
                        == FALSE )
                {
                    fprintf(&(ptrSessionInfo->OutFileHandle), \
                            (const char*)sPtrCliCmdNotFound);
                }

                /* Cli has completed execution of command. */
                inBandUartSetTransferComplete( TRUE );

                /* After each CLI command yield to the other threads. This will
                 * keep the CLI from consuming all the CPU cycles. */
                haliOsThreadRelinquish();
            }
        }
        else
        {
            break;
        }
    }
}

/**
 * @Name: cliCloseSession()
 *
 * @Description: This function closes the CLI session.
 *              CLI session thread is deleted and releases all resources
 *              attached with the CLI session
 * @param PtrCliSessionInfo - Pointer to the CLI Session Information structure
 *
 * @Note This function returns if CLI thread deletion fails without
 *      proceeding further
 *****************************************************************************/
void cliCloseSession( PTR_CLI_SESSION_INFO PtrCliSessionInfo )
{
    HALI_OS_THREAD_STATE thrdState;
    U8 loopCount = 0x0;

    /* Set session active flag to false and wait till the cli thread returns */
    PtrCliSessionInfo->SessionActive = FALSE;

    haliOsThreadInfoGet(PtrCliSessionInfo->CliThreadHandle,
                        NULL,
                        &thrdState,
                        NULL,
                        NULL,
                        NULL,
                        NULL,
                        NULL,
                        NULL);

    /* Wait till the thread is in completed state or max wait time
       exceeds. The maximum wait time is 200 ms otherwise we will forcefully
       terminate & delete the thread.

       We wait here so that the thread serving the client will complete
       gracefully. Earlier the client thread used to get terminated abruptly by
       server thread. This abrupt termination used to create problem if the
       serving client thread is in between of some transfer, thus creating packet
       loss situation.
       E.g., if telnet client thread is transmitting on socket and abruptly the
       connection is closed by other entity. We used to delete the thread without
       waiting for it to complete its current task.
       We need to make sure that the thread comes out of the current task
       (specially when in tight loop). This is accomplished with the help of
       SessionActive flag. Also, we need to make sure that the thread comes out
       of any wait/sleep state. Our client threads are waiting on semaphore
       for receive input. When server receives any disconnect, it fills the
       receive buffer with end of session byte (HALI_EOF), so that the thread
       function returns and thread is in completed state. */

    while( thrdState != HALI_OS_THREAD_STATE_COMPLETED
           && thrdState != HALI_OS_THREAD_STATE_TERMINATED
           && loopCount < 10 )
    {
        haliOsThreadSleep(20);
        haliOsThreadInfoGet(PtrCliSessionInfo->CliThreadHandle,
                            NULL,
                            &thrdState,
                            NULL,
                            NULL,
                            NULL,
                            NULL,
                            NULL,
                            NULL);
        loopCount++;
    }

    /* Terminate the thread. */
    haliOsThreadTerminate(PtrCliSessionInfo->CliThreadHandle);

    /* Delete the thread which was serving this CLI Sesstion */
    haliOsThreadDelete(PtrCliSessionInfo->CliThreadHandle);

    /* Release the Object handle for this particular thread */
    haliOsReleaseObject(PtrCliSessionInfo->CliThreadHandle);

    /* Free the stack memory for this deleted thread */
    if( PtrCliSessionInfo->PtrCliThreadStack )
    {
        free(PtrCliSessionInfo->PtrCliThreadStack);
    }
}


/**
 * @Name:   cliReleaseInit()
 *
 * @Description: This function is exported weakly. Default weak implementation
 *               does nothing. This WEAK function can be overridden by
 *               redefining the function to initialize and register release
 *               build specific CLI commands.
 *              Invoked by cliCoreInit() function
 *
 * @param PtrCliCmdList - Pointer to the CLI Command List
 *               structure.
 *
 *****************************************************************************/
WEAK void cliReleaseInit( PTR_CLI_CMD_LIST PtrCliCmdList )
{
}


/**
 * @Name:   cliDebugInit()
 *
 * @Description: This function is exported weakly. Default weak implementation
 *               does nothing. This WEAK function can be overridden by
 *               redefining the function to initialize and register debug
 *               build specific CLI commands.
 *              Invoked by cliCoreInit() function
 *
 * @param PtrCliCmdList - Pointer to the CLI Command List
 *               structure.
 *
 *****************************************************************************/
WEAK void cliDebugInit( PTR_CLI_CMD_LIST PtrCliCmdList )
{
}


/**
 * @Name:   oemCliInit()
 *
 * @Description: This function is exported weakly. Default weak implementation
 *               does nothing. This WEAK function can be overridden by
 *               redefining the function to initialize and register OEM
 *               specific CLI commands.
 *              Invoked by cliCoreInit() function
 *
 * @param PtrCliCmdList - Pointer to the CLI Command List
 *               structure.
 *
 *****************************************************************************/
WEAK void oemCliInit( PTR_CLI_CMD_LIST PtrCliCmdList )
{
}


/**
 * @Name:   cliFwFaultConsoleCmdRegister()
 *
 * @Description: This function is exported weakly. Default weak implementation
 *               does nothing. This WEAK function can be overridden by
 *               redefining the function to initialize and register fw fault
 *               specific CLI commands.
 *               Invoked by cliFwFaultConsoleInit()
 *
 * @param PtrCliCmdList - Pointer to the CLI Command List
 *               structure.
 *
 *****************************************************************************/
WEAK void cliFwFaultConsoleCmdRegister( PTR_CLI_CMD_LIST PtrCliCmdList )
{
}


/**
 * @Name: cliFwFaultConsoleInit()
 *
 * @Description: This function initializes the Command Line Interface for the
 *               firmware fault handler.
 *
 * @return - Pointer to the session information for the fault handler CLI
 *
 *****************************************************************************/
PTR_CLI_SESSION_INFO cliFwFaultConsoleInit( void )
{
    PTR_CLI_SESSION_INFO ptrSessionInfo = &sCliFwFaultSessionInfo;

    memset( ptrSessionInfo, 0, sizeof(CLI_SESSION_INFO) );

    if ( fileAddRecord(&(ptrSessionInfo->OutFileHandle), STDIO_FW_FAULT_IO_HANDLE) ==
         FILE_STATUS_SUCCESS )
    {
        ptrSessionInfo->fptrDisplyHelp = cliPrintFirstHelpMsg;
        ptrSessionInfo->SessionActive = TRUE;

        /* Register help command - Help command is registered from core itself */
        cliRegisterCommand(( PU8 )sCmdHelp, ( PU8 )sHlpHelp,
                            cliPrintHelp, &ptrSessionInfo->CliCmdList);

        /* Register FW fault console specific CLI commands */
        cliFwFaultConsoleCmdRegister(&ptrSessionInfo->CliCmdList);
    }
    else
    {
        /* When the fault console is not initialized, the firmware should be
         * allowed to continue, but register a warning.
         */
        FW_TRACE0_MACRO(HALI_TRACE_SEVERITY_WARNING,
                        FW_TRACE_STR_FAULT_CONSOLE_NOT_INIT,
                        FW_TRACE_MASK_GENERAL);
    }

    return( ptrSessionInfo );
}


/**
 * @Name:   cliCommandPrompt()
 *
 * @Description: This function reads command using session specific interface function
 *          to read input string and pass the string to cli command string
 *          parser (cliParseCmd())
 *
 * @param  ThreadInput - CLI Thread entry function input parameter
 *          Holds a pointer of type PTR_CLI_SESSION_INFO. Contains CLI session
 *          specific information
 *
 *****************************************************************************/
void cliCommandPrompt( U32 ThreadInput )
{
    PTR_CLI_SESSION_INFO ptrSessionInfo;
    HALI_OS_STATUS retStatus;
    PTR_FILE_RECORD ptrFileRecord;
    U32 postStatusInfo;

    ptrSessionInfo = (PTR_CLI_SESSION_INFO)ThreadInput;

    /* Block till FW initialization completes.*/
    retStatus  = haliDepSyncWait( HALI_DEP_SYNC_FW_INIT_COMPLETE_EVT,
                                  HALI_OS_WAIT_FOREVER );

    haliAssert( HALI_OS_API_SUCCESS(retStatus), FAULT_HAL_DEP_WAIT_FAILURE );

    /* Display header */
    fprintf(&(ptrSessionInfo->OutFileHandle),"\r\n\n%s\r\n\n\
                SAS3 Expander \r\n\r\n\
                \r\n\n%s\r\n\n", gPtrCliPrintHeader, gPtrCliPrintHeader);

    /* Only display POST status if the display help function is
     * cliPrintFirstHelpMsg. For example, if the maximum number of telnet
     * sessions is reached then the display help function is overridden with
     * cliMaxSessionsExceededHandler function. In this case, POST status
     * should not be displayed.
     */
    if( (ptrSessionInfo->fptrDisplyHelp) == &cliPrintFirstHelpMsg )
    {
        /* Firmware init done, check for post errors */
        postStatusInfo = haliPostStatusGet();
        if( HALI_POST_STATUS_SUCCESS != postStatusInfo )
        {
            cliPrintPostInfo(ptrSessionInfo);
        }
    }

    /* Display Initial Help for User. */
    (ptrSessionInfo->fptrDisplyHelp)( ptrSessionInfo );

    ptrFileRecord = fileGetRecord(&(ptrSessionInfo->OutFileHandle));
    if( ptrFileRecord != NULL )
    {
        /* Release firmware initialization resources while
           starting UART CLI so that this is done only once. */
        if( STDIO_UART_IO_HANDLE == ptrFileRecord->Header.InterfaceHandle )
        {
            while( fwInitFreeRsrc() != FW_INIT_RSRC_FREED )
            {
                haliOsThreadSleep(500);
            }
        }
    }

    /* Write Signature for Doorbell */
    haliSetP2HDoorBell(HAL_CLI_SIGNATURE);

    /* While session is active */
    while( ptrSessionInfo->SessionActive == TRUE )
    {
        fprintf(&(ptrSessionInfo->OutFileHandle), "\r\n cmd >");
        /* Clear the choice array which hold the cli command input. */
        memset(ptrSessionInfo->inputString, 0, CLI_MAX_CMD_LINE_LENGTH);

        /* Get user input command from socket. */
        ptrSessionInfo->fptrGetCommandString( ptrSessionInfo->inputString, \
                                              &(ptrSessionInfo->OutFileHandle));
        
        fflush( &(ptrSessionInfo->OutFileHandle) ); /* make sure newline is echoed */

        /* check if command is not empty or CR or EOF then process it. */
        /* EOF is treated as end of CLI session and we don't want to process it.*/
        if( (ptrSessionInfo->inputString[0] != 0x00) &&
            (ptrSessionInfo->inputString[0] != CLI_CR) &&
            ptrSessionInfo->inputString[0] != HALI_EOF )
        {
            if( cliParseCmd(ptrSessionInfo->inputString, ptrSessionInfo)
                    == FALSE )
            {
                fprintf(&(ptrSessionInfo->OutFileHandle), \
                        (const char*)sPtrCliCmdNotFound);
            }
            /* After each CLI command yield to the other threads. This will
             * keep the CLI from consuming all the CPU cycles. */
            haliOsThreadRelinquish();
        }
    }
}


/**
 * @Name:   cliRegisterCommand()
 *
 * @Description: This function registers the command string, the help string
 *               and the Command Handler function into CLI command list.
 *
 * @param PtrCmdName - Command name string
 *
 * @param PtrOneLineHelp - One line help String.
 *
 * @param PtrToFunCall - Command Handler function pointer.
 *
 * @param PtrCliCmdList - Pointer to CLI command List
 *               structure.
 *
 * @return - Status of Command registration - Pass/Fail.
 *
 *****************************************************************************/
CLI_STATUS cliRegisterCommand( PU8 PtrCmdName, PU8 PtrOneLineHelp,
                              CLI_COMMAND_FUNCTION_PTR PtrToFunCall,
                              PTR_CLI_CMD_LIST  PtrCliCmdList )
{
   PTR_CLI_CMD_NODE ptrCliCmdNode;

    /* Allocate memory for new CLI command node */
    if( (ptrCliCmdNode = (PTR_CLI_CMD_NODE)malloc(sizeof(CLI_CMD_NODE))) == NULL )
        return CLI_STATUS_FAILED;

    /* initialize CLI command node */
    ptrCliCmdNode->Command      = PtrCmdName;
    ptrCliCmdNode->OneLineHelp  = PtrOneLineHelp;
    ptrCliCmdNode->PtrToFunCall = PtrToFunCall;

    /* Add new CLI command node at the head of the list */
    ptrCliCmdNode->PtrNext              = PtrCliCmdList->PtrCliCmdListHead;
    PtrCliCmdList->PtrCliCmdListHead    = ptrCliCmdNode;

    /* Increment CLI command count */
    PtrCliCmdList->CliCommandCount++;

    return CLI_STATUS_SUCCESS;

}


/**
 * @Name:   cliParseCmd()
 *
 * @Description: This function parses the command line input and
 *             calls the corresponding command handler function
 *             if parsing is done successfully.
 *
 * @param String - Command line input command.
 *
 * @param PtrSessionInfo - Pointer to the CLI session information
 *                structure.
 *
 * @return - Returns status of command parsing i.e.
 *          TRUE - Command parsed successfully.
 *          FALSE - Command parsing failed.
 *
 *****************************************************************************/
BOOL cliParseCmd( PU8 String, PTR_CLI_SESSION_INFO PtrSessionInfo )
{
    /* Counters for looping, token count & token length resp.*/
    U32     loopCount, tkCount, tkLen;
    PU8     ptrToken = NULL;
    CLI_STATUS      retStatus;

    tkCount = 0;
    tkLen = 0;

    /* Initialize the Command Param array with null string */
    for( loopCount = 0 ; loopCount < CLI_MAX_NUM_OF_TOKENS; loopCount++ )
    {
        PtrSessionInfo->PtrCmdParams[loopCount] = NULL;
    }

    /* Separate out the tokens in the command line input.*/
    ptrToken = (PU8)strtok( (char *)String, CLI_CMD_DELIMS );

    while( ptrToken != NULL )
    {
        tkLen = strlen((const char*)ptrToken);

        if( tkLen >= CLI_MAX_TOKEN_LENGTH || tkCount == CLI_MAX_NUM_OF_TOKENS )
        {
            /*
             * if token length is bigger than maximum token buffer
             * then it will return error.
             */
            return FALSE;
        }

        PtrSessionInfo->PtrCmdParams[tkCount] = (PU8)ptrToken;
        tkCount++;

        ptrToken = (PU8)strtok( NULL, CLI_CMD_DELIMS );
    }

    /*
     * Traverse through CLI command linked list to find a matching command string
     */

    for( PtrSessionInfo->PtrCurCommand = PtrSessionInfo->CliCmdList.PtrCliCmdListHead;
         PtrSessionInfo->PtrCurCommand != NULL;
         PtrSessionInfo->PtrCurCommand = PtrSessionInfo->PtrCurCommand->PtrNext )
    {

        if( strncmp((char *)(PtrSessionInfo->PtrCurCommand->Command),
                    (const char *)PtrSessionInfo->PtrCmdParams[0],
                    CLI_MAX_TOKEN_LENGTH) == 0 )
        {
            PtrSessionInfo->TokenInCmdRcd = tkCount;
            /* Check whether Pointer to function is NULL. */
            if( PtrSessionInfo->PtrCurCommand->PtrToFunCall != NULL )
            {
                /* Call Corresponding callback function. */
                retStatus = PtrSessionInfo->PtrCurCommand->PtrToFunCall(PtrSessionInfo);
                cliErrorHandler( retStatus, PtrSessionInfo );
            }
            break;
        }
    }
    /*
     * Parser status. True - Command found in command database,
     * False - command not found.
     */
    return (PtrSessionInfo->PtrCurCommand != NULL);
}


/**
 *
 * @Name:   cliPrintFirstHelpMsg()
 *
 * @Description: This function displays the first single line help message at the
 *               start of a new CLI session.
 *
 * @param PtrSessionInfo - Pointer to the CLI session information
 *                structure.
 *
 *****************************************************************************/
static CLI_STATUS cliPrintFirstHelpMsg( PTR_CLI_SESSION_INFO PtrSessionInfo )
{
    /* Display Initial Help for User. */
    fprintf(&(PtrSessionInfo->OutFileHandle),
        "\r\n\n Enter 'help' to display a list of commands");

    return CLI_STATUS_SUCCESS;
}


/**
 *
 * @Name:   cliPrintHelp()
 *
 * @Description: This function displays CLI help. It displays
 *             command name and syntax for command.
 *
 * @param PtrSessionInfo - Pointer to the CLI session information
 *                structure.
 *
 * @Note: This function displays help string of the specified command. All registered
 *  commands help strings are displayed if no command is specified as cmd parameter
 *
 * @return result of command execution.
 *
 *****************************************************************************/
CLI_STATUS cliPrintHelp( PTR_CLI_SESSION_INFO PtrSessionInfo )
{
    PTR_CLI_CMD_NODE PtrCmdNode = PtrSessionInfo->CliCmdList.PtrCliCmdListHead;

    /* Display the help Menu. */
    fprintf(&(PtrSessionInfo->OutFileHandle),"\r\n\n%s\r\n\n%s\r\n\n%s\r\n\n",
                gPtrCliPrintHeader,sPtrHelpHeader,gPtrCliPrintHeader);

    /* Print help string of all registred command if no argument is passed */
    if( PtrSessionInfo->PtrCmdParams[1] == NULL )
    {
        for( ; PtrCmdNode ; PtrCmdNode = PtrCmdNode->PtrNext )
        {
            fprintf( &(PtrSessionInfo->OutFileHandle),
            (const char *)(PtrCmdNode->OneLineHelp) );
        }
    }
    else
    {
        for( ; PtrCmdNode ; PtrCmdNode = PtrCmdNode->PtrNext )
        {
            if( strncmp((char *)(PtrCmdNode->Command),
                        (const char *)PtrSessionInfo->PtrCmdParams[1],
                        CLI_MAX_TOKEN_LENGTH) == 0 )
            {
                fprintf( &(PtrSessionInfo->OutFileHandle),
                     (const char *)(PtrCmdNode->OneLineHelp) );
                break;
            }
        }

        /* Print error msg is the specified command not found */
        if( !PtrCmdNode )
            fprintf(&(PtrSessionInfo->OutFileHandle), (const char*)sPtrCliCmdNotFound);
    }

    fprintf(&(PtrSessionInfo->OutFileHandle),"\r\n\n%s", gPtrCliPrintHeader);

    return CLI_STATUS_SUCCESS;
}

/**
 * @Name:   cliGetString()
 *
 * @Description: This function reads user input string using getchar
 *            function. This is the default function used by
 *            CLI core to read input string.
 *
 * @param  PtrInputBuff - memory pointer to store user input string
 *
 * @param PtrOutFileHandle File Handle pointer
 *
 * @return result of command execution.
 *
 *****************************************************************************/
void cliGetString( PU8 PtrInputBuff, FILE *PtrOutFileHandle )
{
    S32     userInput;
    U32     indexCounter = 0;
    BOOL    spaceInput = FALSE;
    static U8 counter = 0;

    PTR_CLI_CMD_HISTORY historyNode = &sCliCmdHistory;
    U8 keyTimes = 0;
    BOOL arrowFlag = FALSE;
    char* lastResult = NULL;
    U8 maxLength = 0;
    
    
    do
    {
        /* Read User Input - one char at a time. */
        userInput = fgetc(PtrOutFileHandle);   
        if( ((indexCounter == 0) && (userInput == CLI_BACKSPACE)) )
        {
            /* Do not echo backspace when command is empty. */
            continue;
        }
        else if( userInput == HALI_EOF )
        {
            PtrInputBuff[0] = (U8)userInput;
            break;
        }
        else if (userInput == CLI_SPACE)
        {
            spaceInput = TRUE;
        }

        /* Echo back the character received */
        if (userInput == CLI_TAB
            && !spaceInput)
        {
            /* Don't echo the first tab */
        }
        else
        {
            fputc(userInput, PtrOutFileHandle);
        }
        /* Read input from command line until user press Enter.*/
        if( (userInput != CLI_CR) &&
            (indexCounter < (CLI_MAX_CMD_LINE_LENGTH) )
          )
        {
            /* If input is Backspace, then clear last character read. */
            if( (userInput == CLI_BACKSPACE) && (indexCounter > 0) )
            {
                indexCounter--;
                PtrInputBuff[indexCounter] = '\0';

                /* This will show the action of back space in user input program */
                fputc(CLI_SPACE, PtrOutFileHandle);
                fputc(CLI_BACKSPACE, PtrOutFileHandle);
            }

            /* Added by XQJ 20180119 for commmand autocomplete. */
            /* 
            ** If input is TAB and no space has been inputed so far,
            ** then try to autocomplete the command.
            */
            else if ((userInput == CLI_TAB) && !spaceInput)
            {
                char * result;
                U8 count = iecCliSearchCommand((const char *)(PtrInputBuff),
                                    indexCounter,
                                    &result,
                                    sCliCmdList);
                if (count == 0)
                {
                    /* Invalid command, just continue. */
                    continue;
                }                    
                else if ( count == 1)
                {
                    /* Move cursor to the start. */
                    while (indexCounter--)
                    {
                        fputc('\b', PtrOutFileHandle);
                    }

                    fputs(result, PtrOutFileHandle);
                    memcpy(PtrInputBuff, result, strlen(result));
                    indexCounter = strlen(result);

                    fputc(CLI_SPACE, PtrOutFileHandle);
                    PtrInputBuff[indexCounter] = CLI_SPACE;
                    indexCounter++;
                    
                    continue;
                }
                else
                {
                    /* Print all the commands that are matched. */
                    PTR_CLI_CMD_NODE PtrCmdNode  = sCliCmdList.PtrCliCmdListHead;
                    
                    for (; PtrCmdNode; PtrCmdNode = PtrCmdNode->PtrNext)
                    {
                        if( strncmp((char *)(PtrCmdNode->Command),
                                    (char *)PtrInputBuff,
                                    indexCounter) == 0 )
                        {
                            fputs("\r\n", PtrOutFileHandle);
                            fputs((const char *)PtrCmdNode->Command,
                                PtrOutFileHandle);
                        }                   
                    }
                    PtrInputBuff[0] = '\0';
                    break;
                }
            }
            /*if input character is arrow key*/
            /* Adding here. */
            else
            {
              
              U8 keyCode = iecCliArrowHandle((U8)userInput);
                /*encounter UP_KEY
                *1.  indexCounter=0 in case of over max of command line length;
                *2   search last command from list.
                *3   print last command use fputs() before move start line;
                *4.  copy command info ptrinputBuffer and indexCounter should
                *    be length of command.
                *    continue ; if key_up again command should be previous than
                *    last command.
                */  
              switch(keyCode)
              {
                    case 2:
                        arrowFlag = TRUE;
                        keyTimes++;
                        fputc('\n', PtrOutFileHandle);
                        break;
                    case 4:
                        arrowFlag = TRUE;
                        keyTimes--;
                        break;
                    default:
                        /* save user input.*/
                        PtrInputBuff[indexCounter] = (U8)userInput;
                        indexCounter++;
                        arrowFlag = FALSE;
                        break;                       
               }   
         if(arrowFlag)
         {    
            if(keyTimes <= counter)
            {
              // echo null if upkeys over max index of command list.
              
            }else
            {
                  keyTimes = counter;
            }
            
            lastResult = cliSearchString(keyTimes, maxLength,
                            sCliCmdHistory.PtrCliCmdListHead, PtrOutFileHandle);
            fflush(PtrOutFileHandle);
            PtrInputBuff[0] = '\0';
            indexCounter = 0;
        }
     }
   }
            
 }while( (userInput != CLI_CR) && (indexCounter < CLI_MAX_CMD_LINE_LENGTH) );
             
    if(arrowFlag && lastResult != NULL)
    {
         
        memcpy(PtrInputBuff, lastResult, strlen(lastResult));
        indexCounter = strlen(lastResult);
    }
     /*also copy into history list.
      * initialize heap of command line meanwhile insert command line;
      * if input CL_CR , it must be ignore;
      */ 
    if(indexCounter >= 1 && arrowFlag == FALSE)
    {   
                 
        if(historyNode)
        {
            counter++;
            maxLength = (maxLength > indexCounter) ? maxLength : indexCounter;
            cliInsertNode(counter, PtrInputBuff, &historyNode);
         }
    } 
    if( indexCounter == (CLI_MAX_CMD_LINE_LENGTH))
    {
        /* Clear the command.*/
        PtrInputBuff[0] = '\0';
        fprintf(PtrOutFileHandle, \
                "\r\n\n Command too long.\r\n");
    }
}

/**
 *
 * @Name: cliInit()
 *
 * @Description: This function Initializes the CLI Core and creates UART based
 *                CLI session.
 *
 * @Note: This function is OBSOLETE and retained only for backward compatibility
 *
 ******************************************************************************/
void cliInit(void)
{
    /* Initialize CLI Core */
    cliCoreInit();

    /* Create UART based CLI session */
    cliUartInitSession();
}


/**
 * @Name:   cliMaxSessionsExceededHandler()
 *
 * @Description: This function display the max sessions exceeded notification, sleeps
 *               for some time to display the message and then closes the session.
 *
 * @param PtrSessionInfo - Pointer to the CLI session information
 *                structure.
 *
 * @return result of command execution.
 *
 * @note This is passed as parameter to the cliCreateSessionEx() function to
 * so to override the cliPrintHelp() function which gets called at the begining
 * of a CLI session thread.
 *****************************************************************************/
CLI_STATUS cliMaxSessionsExceededHandler( PTR_CLI_SESSION_INFO PtrCliSessionInfo )
{
    PTR_FILE_RECORD ptrFileRecord;

    ptrFileRecord = fileGetRecord(&(PtrCliSessionInfo->OutFileHandle));

    if( ptrFileRecord != NULL )
    {
        fprintf(&(PtrCliSessionInfo->OutFileHandle),
            "Maximum number of %s sessions has been exceeded.\r\n ",
            ( ptrFileRecord->Header.InterfaceHandle == STDIO_SSH_IO_HANDLE)?
            "SSH" : "telnet" );

        fflush(&(PtrCliSessionInfo->OutFileHandle));
        haliOsThreadSleep( ( MAX_SESSION_EXCEEDED_MSG_DISPLAY_MS * 1000 ) /
                            haliOsGetMicrosecPerTick() );
    }

    /* Close the client socket. This will further cause the
     * end connection callback to be invoked.
     */
    exitCliSession(PtrCliSessionInfo);

    return CLI_STATUS_SUCCESS;
}


/**
 * @Name:   cliErrorHandler()
 *
 * @Description: This function is a common function to print errors that occur in
 *               cli command execution
 *
 * @param retStatus - Status that was returned from the cli command function
 *
 * @param PtrSessionInfo - Pointer to the CLI session information
 *                structure.
 *****************************************************************************/
void cliErrorHandler( CLI_STATUS retStatus, PTR_CLI_SESSION_INFO PtrCliSessionInfo )
{
     BOOL   syntax = FALSE; /* Flag to determine if syntax of command should be printed.*/
     static const char *errMessage = "";


     /*find what the return status of the command was.*/
     switch( retStatus )
     {
     case CLI_STATUS_SUCCESS:
            errMessage = errorString[CLI_STATUS_SUCCESS];
            break;

     case CLI_STATUS_FAILED:
            errMessage = errorString[CLI_STATUS_FAILED];
            break;

     case CLI_STATUS_SYNTAX:
            errMessage = errorString[CLI_STATUS_SYNTAX];
            syntax = TRUE;
            break;

        case CLI_STATUS_INVALID_WRT_TYPE:
            errMessage = errorString[CLI_STATUS_INVALID_WRT_TYPE];
            syntax = TRUE;
            break;

        case CLI_STATUS_INVALID_ADDRESS:
            errMessage = errorString[CLI_STATUS_INVALID_ADDRESS];
            syntax = TRUE;
            break;

        case CLI_STATUS_INVALID_HIGH_SAS_ADD:
            errMessage = errorString[CLI_STATUS_INVALID_HIGH_SAS_ADD];
            syntax = TRUE;
            break;

        case CLI_STATUS_INVALID_LOW_SAS_ADD:
            errMessage = errorString[CLI_STATUS_INVALID_LOW_SAS_ADD];
            syntax = TRUE;
            break;

        case CLI_STATUS_INVALID_SYNTAX:
            errMessage = errorString[CLI_STATUS_INVALID_SYNTAX];
            syntax = TRUE;
            break;

        case CLI_STATUS_INVALID_PARAMETER:
            errMessage = errorString[CLI_STATUS_INVALID_PARAMETER];
            syntax = TRUE;
            break;

        case CLI_STATUS_INVALID_MEM_LEN:
            errMessage = errorString[CLI_STATUS_INVALID_MEM_LEN];
            syntax = TRUE;
            break;

         case CLI_STATUS_INVALID_ALL_LEN:
             errMessage = errorString[CLI_STATUS_INVALID_ALL_LEN];
             syntax = TRUE;
             break;

        case CLI_STATUS_DATA_OVERFLOW_8:
            errMessage = errorString[CLI_STATUS_DATA_OVERFLOW_8];
            syntax = TRUE;
            break;

        case CLI_STATUS_DATA_OVERFLOW_16:
            errMessage = errorString[CLI_STATUS_DATA_OVERFLOW_16];
            syntax = TRUE;
            break;
        case CLI_STATUS_DATA_OVERFLOW_32:
            errMessage = errorString[CLI_STATUS_DATA_OVERFLOW_32];
            syntax = TRUE;
            break;

        case CLI_STATUS_INVALID_WRT_DATA:
            errMessage = errorString[CLI_STATUS_INVALID_WRT_DATA];
            syntax = TRUE;
            break;

        case CLI_STATUS_THREAD_ERROR:
            errMessage = errorString[CLI_STATUS_THREAD_ERROR];
            syntax = TRUE;
            break;

        case CLI_STATUS_READ_CFG_ERR:
            errMessage = errorString[CLI_STATUS_READ_CFG_ERR];
            syntax = TRUE;
            break;

        case CLI_STATUS_INVALID_PG_ID:
            errMessage = errorString[CLI_STATUS_INVALID_PG_ID];
            syntax = TRUE;
            break;

        case CLI_STATUS_INVALID_REGION:
            errMessage = errorString[CLI_STATUS_INVALID_REGION];
            syntax = TRUE;
            break;

        case CLI_STATUS_ERASE_FAILED:
            errMessage = errorString[CLI_STATUS_ERASE_FAILED];
            syntax = TRUE;
            break;

        case CLI_STATUS_INVALID_ERASE_PARAM:
            errMessage = errorString[CLI_STATUS_INVALID_ERASE_PARAM];
            syntax = TRUE;
            break;

        case CLI_STATUS_RD_MFG_FAILED:
            errMessage = errorString[CLI_STATUS_RD_MFG_FAILED];
            break;

        case CLI_STATUS_RD_LOG_ERROR:
            errMessage = errorString[CLI_STATUS_RD_LOG_ERROR];
            syntax = TRUE;
            break;

        case CLI_STATUS_INVALID_STRING:
            errMessage = errorString[CLI_STATUS_INVALID_STRING];
            syntax = TRUE;
            break;

        case CLI_STATUS_NO_QUOTES:
            errMessage = errorString[CLI_STATUS_NO_QUOTES];
            syntax = TRUE;
            break;

        case CLI_STATUS_LOG_ADD_ERR:
            errMessage = errorString[CLI_STATUS_LOG_ADD_ERR];
            syntax = TRUE;
            break;

        case CLI_STATUS_CLR_LOG_ERR:
            errMessage = errorString[CLI_STATUS_CLR_LOG_ERR];
            syntax = TRUE;
            break;

        case CLI_STATUS_CLEAR_TRACE_ERR:
            errMessage = errorString[CLI_STATUS_CLEAR_TRACE_ERR];
            syntax = TRUE;
            break;

        case CLI_STATUS_ENTER_HEX:
            errMessage = errorString[CLI_STATUS_ENTER_HEX];
            syntax = TRUE;
            break;

        case CLI_STATUS_TRACE_ERR:
            errMessage = errorString[CLI_STATUS_TRACE_ERR];
            syntax = TRUE;
            break;

        case CLI_STATUS_TRACE_VALID_RANGE:
            errMessage = errorString[CLI_STATUS_TRACE_VALID_RANGE];
            syntax = TRUE;
            break;

        case CLI_STATUS_WRT_FAILED:
            errMessage = errorString[CLI_STATUS_WRT_FAILED];
            syntax = TRUE;
            break;

        case CLI_STATUS_INVALID_TRCMASK:
            errMessage = errorString[CLI_STATUS_INVALID_TRCMASK];
            syntax = TRUE;
            break;

        case CLI_STATUS_INVALID_SUBSYSTEM:
            errMessage = errorString[CLI_STATUS_INVALID_SUBSYSTEM];
            syntax = TRUE;
            break;

        case CLI_STATUS_BYTE_ERR:
            errMessage = errorString[CLI_STATUS_BYTE_ERR];
            syntax = TRUE;
            break;

        case CLI_STATUS_INVALID_CHANNEL:
            errMessage = errorString[CLI_STATUS_INVALID_CHANNEL];
            syntax = TRUE;
            break;

        case CLI_STATUS_MIR_EN_DISEN_FAIL:
            errMessage = errorString[CLI_STATUS_MIR_EN_DISEN_FAIL];
            break;

        case CLI_STATUS_INVALID_RX_PHY:
            errMessage = errorString[CLI_STATUS_INVALID_RX_PHY];
            break;

        case CLI_STATUS_INVALID_TX_PHY:
            errMessage = errorString[CLI_STATUS_INVALID_TX_PHY];
            break;

        case CLI_STATUS_INVALID_SRC_PHY:
            errMessage = errorString[CLI_STATUS_INVALID_SRC_PHY];
            break;

        case CLI_STATUS_INVALID_MIR_PHY:
            errMessage = errorString[CLI_STATUS_INVALID_MIR_PHY];
            break;

        case CLI_STATUS_INSUFF_MEM:
            errMessage = errorString[CLI_STATUS_INSUFF_MEM];
            break;

        case CLI_STATUS_UART_ONLY:
            errMessage = errorString[CLI_STATUS_UART_ONLY];
            break;

        case CLI_STATUS_ETC_DIABLED:
            errMessage = errorString[CLI_STATUS_ETC_DIABLED];
            break;

        case CLI_STATUS_ETC_DISABLE_FAIL:
            errMessage = errorString[CLI_STATUS_ETC_DISABLE_FAIL];
            break;

        case CLI_STATUS_INVALID_TIMEOUT:
            errMessage = errorString[CLI_STATUS_INVALID_TIMEOUT];
            syntax = TRUE;
            break;

        case CLI_STATUS_INVALID_MODE:
            errMessage = errorString[CLI_STATUS_INVALID_MODE];
            syntax = TRUE;
            break;

        case CLI_STATUS_INVALID_INTERFACE:
            errMessage = errorString[CLI_STATUS_INVALID_INTERFACE];
            syntax = TRUE;
            break;

        case CLI_STATUS_NO_SGX_DEVICES:
            errMessage = errorString[CLI_STATUS_NO_SGX_DEVICES];
            break;

        case CLI_STATUS_INVALID_PORT:
            errMessage = errorString[CLI_STATUS_INVALID_PORT];
            syntax = TRUE;
            break;

        case CLI_STATUS_INVALID_CON_RATE:
            errMessage = errorString[CLI_STATUS_INVALID_CON_RATE];
            syntax = TRUE;
            break;

        case CLI_STATUS_XFER_FAILED:
            errMessage = errorString[CLI_STATUS_XFER_FAILED];
            break;

        case CLI_STATUS_INVALID_FILE_LEN:
            errMessage = errorString[CLI_STATUS_INVALID_FILE_LEN];
            syntax = TRUE;
            break;

        case CLI_STATUS_INVALID_FUNC_REQ:
            errMessage = errorString[CLI_STATUS_INVALID_FUNC_REQ];
            syntax = TRUE;
            break;

        case CLI_STATUS_INVALID_BUF_ID:
            errMessage = errorString[CLI_STATUS_INVALID_BUF_ID];
            syntax = TRUE;
            break;

        case CLI_STATUS_CREATE_THRD_FAILED:
            errMessage = errorString[CLI_STATUS_CREATE_THRD_FAILED];
            break;

        case CLI_STATUS_FLASH_ERROR:
            errMessage = errorString[CLI_STATUS_FLASH_ERROR];
            break;

        case CLI_STATUS_NO_FLASH:
            errMessage = errorString[CLI_STATUS_NO_FLASH];
            break;

        case CLI_STATUS_SRC_GROUP_ERR:
            errMessage = errorString[CLI_STATUS_SRC_GROUP_ERR];
            syntax = TRUE;
            break;

        case CLI_STATUS_INVALID_SEPRM_DATA:
            errMessage = errorString[CLI_STATUS_INVALID_SEPRM_DATA];
            break;

        case CLI_STATUS_RD_SEPRM_ERROR:
            errMessage = errorString[CLI_STATUS_RD_SEPRM_ERROR];
            break;

        case CLI_STATUS_INVALID_PARAM_NUM:
            errMessage = errorString[CLI_STATUS_INVALID_PARAM_NUM];
            syntax = TRUE;
            break;

        case CLI_STATUS_ILLEGAL_ARG:
            errMessage = errorString[CLI_STATUS_ILLEGAL_ARG];
            syntax = TRUE;
            break;

        case CLI_STATUS_ILLEGAL_RESET_ARG:
            errMessage = errorString[CLI_STATUS_ILLEGAL_RESET_ARG];
            break;

        case CLI_STATUS_INVALID_PKT_SIZE:
            errMessage = errorString[CLI_STATUS_INVALID_PKT_SIZE];
            break;

        case CLI_STATUS_INVALID_DELAY_ARG:
            errMessage = errorString[CLI_STATUS_INVALID_DELAY_ARG];
            syntax = TRUE;
            break;

        case CLI_STATUS_INVALID_PATTERN_ARG:
            errMessage = errorString[CLI_STATUS_INVALID_PATTERN_ARG];
            syntax = TRUE;
            break;

        case CLI_STATUS_INFO_NOT_AVAILABLE:
            errMessage = errorString[CLI_STATUS_INFO_NOT_AVAILABLE];
            break;

        case CLI_STATUS_ERROR_PRINTED:
            errMessage = errorString[CLI_STATUS_ERROR_PRINTED];
            break;

        case CLI_STATUS_MEM_DL_FAILED:
            errMessage = errorString[CLI_STATUS_MEM_DL_FAILED];
            break;

        case CLI_STATUS_MEM_UL_FAILED:
            errMessage = errorString[CLI_STATUS_MEM_UL_FAILED];
            break;

        case CLI_STATUS_FW_DL_FAILED:
            errMessage = errorString[CLI_STATUS_FW_DL_FAILED];
            break;

        case CLI_STATUS_FW_UL_FAILED:
            errMessage = errorString[CLI_STATUS_FW_UL_FAILED];
            break;

        case CLI_STATUS_INVALID_BUF_OFFSET:
            errMessage = errorString[CLI_STATUS_INVALID_BUF_OFFSET];
            syntax = TRUE;
            break;

        case CLI_STATUS_INVALID_MEM_ADDRESS:
            errMessage = errorString[CLI_STATUS_INVALID_MEM_ADDRESS];
            syntax = TRUE;
            break;

        case CLI_STATUS_INVALID_FILENAME:
            errMessage = errorString[CLI_STATUS_INVALID_FILENAME];
            syntax = TRUE;
            break;

        case CLI_STATUS_INVALID_IP_ADDRESS:
            errMessage = errorString[CLI_STATUS_INVALID_IP_ADDRESS];
            syntax = TRUE;
            break;

        case CLI_STATUS_INVALID_FILE_SIZE:
            errMessage = errorString[CLI_STATUS_INVALID_FILE_SIZE];
            syntax = TRUE;
            break;

        case CLI_STATUS_INVALID_PHYTEST_MODE:
            errMessage = errorString[CLI_STATUS_INVALID_PHYTEST_MODE];
            syntax = TRUE;
            break;

        case CLI_STATUS_INVALID_PHYTEST_PATTERN:
            errMessage = errorString[CLI_STATUS_INVALID_PHYTEST_PATTERN];
            syntax = TRUE;
            break;

        case CLI_STATUS_INVALID_PHYTEST_RATE:
            errMessage = errorString[CLI_STATUS_INVALID_PHYTEST_RATE];
            syntax = TRUE;
            break;

        case CLI_STATUS_INVALID_PHYTEST_DURATION:
            errMessage = errorString[CLI_STATUS_INVALID_PHYTEST_DURATION];
            syntax = TRUE;
            break;

        case CLI_STATUS_INVALID_PHYTEST_PHYMASK:
            errMessage = errorString[CLI_STATUS_INVALID_PHYTEST_PHYMASK];
            syntax = TRUE;
            break;

        case CLI_STATUS_MALLOC_FAILED:
            errMessage = errorString[CLI_STATUS_MALLOC_FAILED];
            break;

        case CLI_STATUS_INVALID_FW_IMAGE:
            errMessage = errorString[CLI_STATUS_INVALID_FW_IMAGE];
            break;

        default:
            break;
    }

    /*print required message from the command return.*/
    fprintf(&(PtrCliSessionInfo->OutFileHandle), "\r\n%s\r\n", errMessage);

    /*verify syntax of the command should be printed.*/
    if ( syntax == TRUE )
    {
        /*print one line help for command syntax in event of invalid command input.*/
        fprintf(&(PtrCliSessionInfo->OutFileHandle), "\r\n%s\r\n%s\r\n",
                errorString[CLI_STATUS_SYNTAX], PtrCliSessionInfo->PtrCurCommand->OneLineHelp);
    }
}







