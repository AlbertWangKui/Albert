/***************************************************************************
 *                                                                         *
 *  Copyright 2018- Inventec.  All rights reserved.                        *
 *                                                                         *
 ***************************************************************************
 *
 *           Name:  iecCli.c
 *          Title:  IEC CLI API
 *     Programmer:  Xie Qian-jun
 *  Creation Date:  January 22, 2018
 *
 *  Version History
 *  ---------------
 *
 *  Date      Who   Description
 *  --------  ---   -------------------------------------------------------
 *  7 /01/19  AW     add iecdisplaysmartwarrantythreshold() to test threshold
 *  06/01/19  AW     Added  slot driver id to send read warranty threshold 
 *                   for special ata in iecSmartReadWaTh()
 *  09/13/18  XQJ    Added project-specific drive id to phy handler in
 *                   iecAtaDevTemp().
 *  08/28/18  XQJ    Added detection of IEC_PHY_SPEED_DISABLED in iecCliSasPort().
 *  08/23/18  XQJ    Added iecCliEnclosure().
 *                   Synced changes with Starmie code.
 *  05/13/18  XQJ    Fixed comments for iecCliInit().
 *                   Changed iecCliDownl() based on the changes in iecUpdate.c
 *  04/15/18  D D    Added Fan,Redriver,Fru,psu cli function.
 *  02/08/18  XQJ    Added Doxygen tag at the end of file.
 *                   Added iecLog, iecFantable, iecIstwi commands.
 *  01/22/18  XQJ    Initial version.
 *
 *
 * Description
 * ------------
 *  This file is the source file of IEC CLI modules.
 *
 *-------------------------------------------------------------------------
 */


#include "iec.h"
#include "xmodem.h"
#include "cliUart.h"
#include "cliTelnet.h"
/** @addtogroup  iecCli CLI API
 *  @{ */


#define IEC_CLI_PRINT_HEADER  "========================================"\
                              "=============================================="

#define IEC_CLI_SGPIO_LED_EXT_INT_PAT_HEADER_1    "LOGICAL    PHYSICAL    EXTRN_LED_GRP_1         EXTRN_LED_GRP_2         EXTRN_LED_GRP_3"
#define IEC_CLI_SGPIO_LED_EXT_INT_PAT_HEADER_2    "PHY        PHY         PATTERN     INV         PATTERN     INV         PATTERN     INV"
#define IEC_CLI_SGPIO_LED_EXT_INT_PAT_HEADER_3    "ID         ID          SELECT      Y/N         SELECT      Y/N         SELECT      Y/N"


/*
** extern functions
*/

extern U8 arbokDriveIdToPhysicalPhyId(U8 DriveId);


/*
** CLI Handler Function Prototypes
*/

CLI_STATUS iecCliDebug(PTR_CLI_SESSION_INFO PtrSessionInfo);
                              
CLI_STATUS iecCliGPIO(PTR_CLI_SESSION_INFO PtrSessionInfo);
                              
CLI_STATUS iecCliSasPort(PTR_CLI_SESSION_INFO PtrSessionInfo);
                              
CLI_STATUS iecCliSasAddr(PTR_CLI_SESSION_INFO PtrSessionInfo);
                              
CLI_STATUS iecCliLog(PTR_CLI_SESSION_INFO PtrSessionInfo);
                                                            
CLI_STATUS iecCliIstwi(PTR_CLI_SESSION_INFO PtrSessionInfo);

CLI_STATUS iecCliTemperatureSensor(PTR_CLI_SESSION_INFO PtrSessionInfo);
                              
CLI_STATUS iecCliFwInfo(PTR_CLI_SESSION_INFO PtrSessionInfo);

CLI_STATUS iecCliSmartReadData(PTR_CLI_SESSION_INFO PtrSessionInfo);
                              
#ifndef PRODUCTION_RELEASE
CLI_STATUS iecCliTest(PTR_CLI_SESSION_INFO PtrSessionInfo);
#endif

CLI_STATUS iecCliAtaDevTemperature(PTR_CLI_SESSION_INFO PtrSessionInfo);

#ifdef ATA_ENABLE_THRESHOLD
CLI_STATUS iecCliSmartReadWaTh(PTR_CLI_SESSION_INFO PtrSessionInfo);
//void iecCliDisplayWarrantyThreshold(PTR_CLI_SESSION_INFO PtrSessionInfo, PU8 PtrData);

#endif

CLI_STATUS iecCliSgpio(PTR_CLI_SESSION_INFO PtrSessionInfo);

CLI_STATUS iecCliEnclosure(PTR_CLI_SESSION_INFO PtrSessionInfo);

/*
** CLI Info structures
*/


const CLI_CMD_INFO gCliCmdIecDebug = {
                                "iecDebug",
                                "    show/set debug level     iecDebug [level <info|warning|critical> | module <mid> <disable|enable> >\r\n"
                                "    and enabled modules      - With no arguments show current debug level\r\n"
                                "                               and enabled modules\r\n",
                                 iecCliDebug
                            };

const CLI_CMD_INFO gCLiCmdIecGpio = {
                               "iecGPIO",
                                "    show/set gpio dir/val       iecGPIO [GPIO] [set <dir <in|out> |val <0|1> >]\r\n"
                                "                             - With no arguments show current settings\r\n",
                                iecCliGPIO
                            };

const CLI_CMD_INFO gCLiCmdIecSgpio = {
                                "iecSgpio",
                                "    toggle sgpio dout value     iecSgpio [log_phy] [loc|err] [on|off|blink]\r\n",
                                iecCliSgpio
                            };

                               
const CLI_CMD_INFO gCLiCmdIecSasPort = {
                               "iecSasPort",
                                "    show/set sas port          iecSasPort [SasPort] [PortOpCode]\r\n"
                                "                             - With no arguments show current settings\r\n"
                                "                             - PortOpCode 0 noop, 1 link reset, 2 hard reset, 3 disable\r\n",
                                iecCliSasPort
                            };

const CLI_CMD_INFO gCLiCmdIecSasAddr = {
                               "iecSasAddr",
                                "    show/set sas address       iecSasAddr [<High[H] Low[H]>]\r\n"
                                "                             - With no arguments show current settings\r\n",
                                iecCliSasAddr
                            };
                               
const CLI_CMD_INFO gCLiCmdIecLog = {
                               "iecLog",
                                "    show/add iec log           iecLog [logstring]\r\n"
                                "                             - With no arguments show all iec logs\r\n",
                                iecCliLog
                            };

const CLI_CMD_INFO gCLiCmdIecIstwi = {
                               
                                "iecIstwi",
                                "    test istwi interface        iecIstwi <scan> <bus_id>\r\n"
                                "                             -scan bus to find slave device\r\n",
                                iecCliIstwi
                            };

const CLI_CMD_INFO gCliCmdIecTemp = {
                                "iecTemp",
                                "   show temperature sensor values iecTemp\r\n",
                                iecCliTemperatureSensor
                            };
                                
const CLI_CMD_INFO gCliCmdIecFwInfo = {
                                "iecFwInfo",
                                "    show boot info                 iecFwinfo <toggle>\r\n",
                                iecCliFwInfo
                            };

const CLI_CMD_INFO gCliCmdIecSmartReadData = {
                                "iecSmartRead",
                                "    SMART READ DATA for SATA drive  iecSmartRead <phy_id>\r\n",
                                iecCliSmartReadData
                            };


const CLI_CMD_INFO gCliCmdIecAtaDevTemp = {
                                "iecAtaDevTemp",
                                "    Read ATA Device temperature     iecAtaDevTemp <drive_id>\r\n"
                                "                             - With no arguments show cached tempearture\r\n",
                                iecCliAtaDevTemperature
                    };

const CLI_CMD_INFO gCliCmdIecEncl= {
                                "iecEnclosure",
                                "    Enclosure management            iecEnclosure <show>\r\n",
                                iecCliEnclosure
                    };


#ifndef PRODUCTION_RELEASE
const CLI_CMD_INFO gCliCmdIecTest= {
                                "iecTest",
                                "\r\n",
                                iecCliTest
                            };
#endif

#ifdef ATA_ENABLE_THRESHOLD
const CLI_CMD_INFO gCliCmdSmartReadData={
                            "iecSmartReadWaTh",
								"SMART READ DATA for SATA drive  iecSmartRead <phy_id>\r\n",
							 iecCliSmartReadWaTh	 
                           };
#endif

/*
** IEC CLI command list
*/

static const CLI_CMD_INFO *sPtrIecCliCmdList[] =	{
                                                        &gCliCmdIecDebug,
                                                        &gCLiCmdIecGpio,
                                                        &gCLiCmdIecSasAddr,
                                                        &gCLiCmdIecSasPort,
                                                        &gCLiCmdIecLog,
                                                        &gCLiCmdIecIstwi,
                                                        &gCliCmdIecTemp,
                                                        &gCliCmdIecFwInfo,
                                                        &gCliCmdIecSmartReadData,
                                                        &gCliCmdIecAtaDevTemp,
                                                        &gCLiCmdIecSgpio,
                                                        &gCliCmdIecEncl,
                                                        #ifndef PRODUCTION_RELEASE
                                                        &gCliCmdIecTest,
                                                        #endif
														#ifdef ATA_ENABLE_THRESHOLD
														&gCliCmdSmartReadData
														#endif
                                                    };


/**
 *
 * @Name:   iecCliDebug()
 *
 * @Description: This command change the debug level and enable modules for
 *               debug printf.
 *
 * @param PtrSessionInfo - Pointer to the CLI session information structure.
 *
 * @return result of command execution.
 *
 *****************************************************************************/
CLI_STATUS iecCliDebug( PTR_CLI_SESSION_INFO PtrSessionInfo )
{
    S32 retVal;
    U8 invalidChar;
    U32 midNum;
    
    if( PtrSessionInfo->TokenInCmdRcd == 1 )
    {
        U8 midNum;
        
        /* By default, show current debug level and enabled modules. */
        CLI_PRINTF("\r\nCurrent debug level:  ");

        switch (gDebugLevel)
        {
            case IEC_INFO:
                CLI_PRINTF("info-3\r\n");
                break;
            case IEC_WARNING:
                CLI_PRINTF("warning-2\r\n");
                break;
            case IEC_CRITICAL:
                CLI_PRINTF("critical-1\r\n");
                break;
            default:
                CLI_PRINTF("Unknown-%d", gDebugLevel);
                break;
        }
        
        CLI_PRINTF("Current enabled modules: \r\n");

        for( midNum = 0; midNum < IEC_MID_MAX; midNum++ )
        {
            CLI_PRINTF("%d-%s: ", midNum, iecModulesName[midNum]);
            if( CHECK_MID_DEBUG(midNum) )
            {
                CLI_PRINTF("enabled.\r\n");
            }
            else
            {
                CLI_PRINTF("disabled.\r\n");
            }
        }

        return CLI_STATUS_SUCCESS;
    }
    else if( PtrSessionInfo->TokenInCmdRcd == 3 )
    {
        if( strcmp((const char *)PtrSessionInfo->PtrCmdParams[1], "level") == 0 )
        {
            /* Get the Debug level info/warning/critical and set the global var.*/
            if (CLI_PARAM_STRCMP(2, "info") == 0)
            {
                gDebugLevel = IEC_INFO;
            }
            else if (CLI_PARAM_STRCMP(2, "warning") == 0)
            {
                gDebugLevel = IEC_WARNING;
            }
            else if ((CLI_PARAM_STRCMP(2, "critical") == 0))
            {
                gDebugLevel = IEC_CRITICAL;
            }
            else
            {
                return CLI_STATUS_INVALID_PARAMETER;
            }

            return CLI_STATUS_SUCCESS;
        }
    }
    else if( PtrSessionInfo->TokenInCmdRcd == 4 )
    {
        if( strcmp((const char *)PtrSessionInfo->PtrCmdParams[1], "module") == 0 )
        {
            /* Get the module id.*/
            retVal = sscanf((const char *)PtrSessionInfo->PtrCmdParams[2], "%d%c",
                            &midNum, &invalidChar);

            if( (retVal == 1) && 
                (midNum < IEC_MID_MAX) )
            {
                /* Disable or enable? */
                if( strcmp((const char *)PtrSessionInfo->PtrCmdParams[3], "enable") == 0 )
                {
                    ENABLE_MID_DEBUG(midNum);
                }
                else if( strcmp((const char *)PtrSessionInfo->PtrCmdParams[3], "disable") == 0 ) 
                {
                    DISABLE_MID_DEBUG(midNum);
                }
                else
                {
                    return CLI_STATUS_INVALID_PARAMETER;
                }
                
                return CLI_STATUS_SUCCESS;
            }
            else
            {
                return CLI_STATUS_INVALID_PARAMETER;
            }

        }
        else
        {
            return CLI_STATUS_INVALID_PARAMETER;
        }
    }

    return CLI_STATUS_INVALID_PARAM_NUM;

}

/**
 * @Name:   iecCliSasPort()
 *
 * @Description:    This CLI API performs sas port operations
 *
 * @param PtrSessionInfo - Pointer to the CLI session information
 *                structure.
 *
 * @return result of command execution.
 *
 *****************************************************************************/

CLI_STATUS iecCliSasPort(PTR_CLI_SESSION_INFO PtrSessionInfo)
{
    U32 portIndex;
    S32 retVal;
    U8 invalidChar;
    U32 portOp;
    U8 portNum = iecSasPortGetPortNum();
    IEC_SAS_PORT_STATUS portStatus;
    PTR_IEC_SAS_PORT_CFG ptrPortCfg;

    if (PtrSessionInfo->TokenInCmdRcd == 1)
    {
        CLI_PRINTF("\r\nPort    PHY     Link    Rate\r\n");
        for (portIndex = 0; portIndex < portNum; portIndex ++)
        {
            U8 phyIndex;
        
            ptrPortCfg = iecSasPortReadCfg(portIndex);
            iecSasPortReadStatus(portIndex, &portStatus);
            for (phyIndex = 0; phyIndex < IEC_SAS_PORT_PHY_CNT; phyIndex++)
            {
                if (portStatus.PortPhyLinkRate[phyIndex]
                    != IEC_PHY_SPEED_DISABLED)
                {
                    CLI_PRINTF("%-8d%-8d%-8d%-8d\r\n", portIndex,
                            ptrPortCfg->PortPhyNum[phyIndex],
                            portStatus.PortPhyLinkStatus[phyIndex],
                            portStatus.PortPhyLinkRate[phyIndex]);
                }
                else
                {
                    CLI_PRINTF("%-8d%-8d%-8d%-8s\r\n", portIndex,
                            ptrPortCfg->PortPhyNum[phyIndex],
                            portStatus.PortPhyLinkStatus[phyIndex],
                            "disabled");                    
                }
           }
        }

        return CLI_STATUS_SUCCESS;
    }
    else if (PtrSessionInfo->TokenInCmdRcd == 3)
    {
        retVal = sscanf((const char *)PtrSessionInfo->PtrCmdParams[1], "%d%c",
                        &portIndex, &invalidChar);
        if ( (retVal == 1) 
            && (portIndex < portNum))
        {
            retVal = sscanf((const char *)PtrSessionInfo->PtrCmdParams[2], "%d%c",
                            &portOp, &invalidChar);
            if ((retVal == 1)
                && portOp <= HALI_PHY_OP_POWER_UP)
            {
                iecSasPortOperate(portIndex, portOp);

                return CLI_STATUS_SUCCESS;
            }
        }

       return CLI_STATUS_INVALID_PARAMETER;
    }
    
    return CLI_STATUS_INVALID_PARAM_NUM;
}

/**
 *
 * @Name:   iecCliSgpioPrintPattern()
 *
 * @Description: This function displays the pattern ID string for Internal and
 *               External Group. This function is the helper function for
 *               iecCliSgpio().
 *
 * @param PtrSessionInfo - Pointer to the CLI session input parameters.
 *
 * @param Data Pattern IDs for Internal and External Group
 *
 * @param Type 1 : External grp data  2 : Internal grp data
 *
 *****************************************************************************/

static void iecCliSgpioPrintPattern( PTR_CLI_SESSION_INFO PtrSessionInfo,
        U32 Data, U32 Type )
{
    U32 tempData;
    char str[14] = {0};

    /* External and Internal Port Config Group
     * Type 1 : External Port config
     * Type 2 : Internal Port config
     */
    if( (1 == Type) || (2 == Type) )
    {
        /* Check for Pattern IDs for Internal and External Group */
        switch( Data )
        {
            case HALI_LED_EXT_PHY_PATTERN_OFF:
            {
                strcpy(str, "OFF");
                break;
            }
            case HALI_LED_EXT_PHY_PATTERN_ON:
            {
                strcpy(str, "ON");
                break;
            }
            /* Generic Blink Pattern Generator for internal and
             * external groups
             */
            case HALI_LED_EXT_PHY_USER_DEFINED_PATTERN_0:
            case HALI_LED_EXT_PHY_USER_DEFINED_PATTERN_1:
            case HALI_LED_EXT_PHY_USER_DEFINED_PATTERN_2:
            case HALI_LED_EXT_PHY_USER_DEFINED_PATTERN_3:
            case HALI_LED_EXT_PHY_USER_DEFINED_PATTERN_4:
            case HALI_LED_EXT_PHY_USER_DEFINED_PATTERN_5:
            case HALI_LED_EXT_PHY_USER_DEFINED_PATTERN_6:
            case HALI_LED_EXT_PHY_USER_DEFINED_PATTERN_7:
            {
                tempData = Data - HALI_LED_EXT_PHY_USER_DEFINED_PATTERN_0;
                snprintf(str, 6, "PAT_%d", tempData);
                break;
            }
            case HALI_LED_EXT_PHY_ACTIVITY:
            {
                strcpy(str, "PHY_ACT");
                break;
            }
            case HALI_LED_EXT_PHY_ACTIVITY_TRAILING_EDGE:
            {
                strcpy(str, "TRL_EDGE");
                break;
            }
            case HALI_LED_EXT_PHY_ACTIVITY_INVERTED_FAULT_COMBO:
            {
                strcpy(str, "COMBO");
                break;
            }
            case HALI_LED_EXT_PHY_FAULT:
            {
                if( 1 == Type )
                {
                    strcpy(str, "FAULT");
                }
                else
                {
                    strcpy(str, "0");
                }
                break;
            }
            case HALI_LED_EXT_PHY_WIDE_PORT_PARTIAL_UP:
            {
                if( 1 == Type )
                {
                    strcpy(str, "WP_PART_UP");
                }
                else
                {
                    strcpy(str, "0");
                }
                break;
            }
            case HALI_LED_EXT_PHY_WIDE_PORT_UP:
            {
                if( 1 == Type )
                {
                    strcpy(str, "WP_UP");
                }
                else
                {
                    strcpy(str, "0");
                }
                break;
            }
            case HALI_LED_EXT_PHY_WIDE_PORT_ACTIVITY:
            {
                if( 1 == Type )
                {
                    strcpy(str, "WP_ACT");
                }
                else
                {
                    strcpy(str, "0");
                }
                break;
            }
            case HALI_LED_EXT_PHY_SGPIO_DATA_BANK_1:
            {
                if( 1 == Type )
                {
                    strcpy(str, "BANK_1");
                }
                else
                {
                    strcpy(str, "0");
                }
                break;
            }
            case HALI_LED_EXT_PHY_SGPIO_DATA_BANK_2:
            {
                if( 1 == Type )
                {
                    strcpy(str, "BANK_2");
                }
                else
                {
                    strcpy(str, "0");
                }
                break;
            }
            case HALI_LED_EXT_PHY_EXT_0:
            {
                if( 1 == Type )
                {
                    strcpy(str, "PHY_EXT_0");
                }
                else
                {
                    strcpy(str, "DBGACK");
                }
                break;
            }
            case HALI_LED_EXT_PHY_EXT_1:
            case HALI_LED_EXT_PHY_EXT_2:
            case HALI_LED_EXT_PHY_EXT_3:
            case HALI_LED_EXT_PHY_EXT_4:
            case HALI_LED_EXT_PHY_EXT_5:
            case HALI_LED_EXT_PHY_EXT_6:
            case HALI_LED_EXT_PHY_EXT_7:
            {
                if( 1 == Type )
                {
                    tempData = Data - HALI_LED_EXT_PHY_EXT_0;
                    snprintf(str, 10, "PHY_EXT_%d", tempData );
                }
                else
                {
                    strcpy(str, "TEST_ACS");
                }
                break;
            }
            default:
            {
                strcpy(str, "0");
                break;
            }
        }
        fprintf(&(PtrSessionInfo->OutFileHandle), "%-13s", str);
    }
}
        
/**
 *
 * @Name:   iecCliSgpioPrintGrpPattern()
 *
 * @Description: This function displays the Group pattern information. This
 *               function is the helper function for iecCliSgpio().
 *
 * @param PtrSessionInfo - Pointer to the CLI session input parameters.
 *
 *****************************************************************************/

static void iecCliSgpioPrintGrpPattern( PTR_CLI_SESSION_INFO PtrSessionInfo )
{
    U32 logicalPhyId = 0;
    U32 physicalPhyId = 0;
    HALI_LED_PHY_GROUP grpcount;
    HALI_LED_GPIO_STATUS status = HALI_LED_GPIO_SUCCESS;
    HALI_LED_PATTERN_IDS patternId;
    BOOL invertPattern;
    U8 invChar;

    /* Display External and Internal Group Pattern Headers */
    CLI_PRINTF("\r\n\nExternal Group Pattern :-\r\n");
    CLI_PRINTF(IEC_CLI_PRINT_HEADER "\r\n");
    CLI_PRINTF(IEC_CLI_SGPIO_LED_EXT_INT_PAT_HEADER_1 "\r\n");
    CLI_PRINTF(IEC_CLI_SGPIO_LED_EXT_INT_PAT_HEADER_2 "\r\n");
    CLI_PRINTF(IEC_CLI_SGPIO_LED_EXT_INT_PAT_HEADER_3 "\r\n");
    CLI_PRINTF(IEC_CLI_PRINT_HEADER);

    /* Display External and Internal Group Patterns */
    for( logicalPhyId = 0;
         logicalPhyId < HALI_EXP_NUM_PHYS;
         logicalPhyId++ )
    {
        physicalPhyId = haliPhyRemapLogicalToPhysical(logicalPhyId);
        CLI_PRINTF("\r\n%02d         %02d          ", logicalPhyId, physicalPhyId);

        for( grpcount = HALI_LED_GROUP_1_EXTERNAL;
             grpcount < HALI_LED_GROUP_INTERNAL;
             grpcount++ )
        {
            /* External LED Group 1 Configuration */
            status = haliLedGetPatternId( grpcount,
                    physicalPhyId, &patternId, &invertPattern );
            if( HALI_LED_GPIO_SUCCESS == status )
            {
                /* Print corresponding pattern ID */
                iecCliSgpioPrintPattern( PtrSessionInfo,
                        patternId.ExtPattern, 1 );

                /* Print invert pattern flag */
                invChar = ( TRUE == invertPattern ) ? 'Y' : 'N';
                CLI_PRINTF("%c          ", invChar );
            }
            else
            {
                CLI_PRINTF("---          " );
                CLI_PRINTF("-          " );
            }
        }
    }

    CLI_PRINTF("\r\n%s\r\n", IEC_CLI_PRINT_HEADER);
}  
/** 
 *
 * @Name:   iecCliSgpio()
 *
 * @Description: This command toggles sgpio dout output.
 *
 * @param PtrSessionInfo - Pointer to the CLI session information structure.
 *
 * @return result of command execution.
 *
 *****************************************************************************/

CLI_STATUS iecCliSgpio(PTR_CLI_SESSION_INFO PtrSessionInfo)
{
    U32 logicalPhyNum;
    U8 invalidChar;
    IEC_SGPIO_DOUT_BIT doutBit;

    if (PtrSessionInfo->TokenInCmdRcd == 1)
    {
        iecCliSgpioPrintGrpPattern(PtrSessionInfo);

        return CLI_STATUS_SUCCESS; 
    }
    else if (PtrSessionInfo->TokenInCmdRcd != 4)
    {
        return CLI_STATUS_INVALID_PARAM_NUM;
    }
    
    CLI_PARAM_PARSE_U32(1,
                    &logicalPhyNum,
                    &invalidChar);

    if (logicalPhyNum > HALI_EXP_NUM_PHYS)
    {
        return CLI_STATUS_INVALID_PARAMETER;
    }

    if (CLI_PARAM_STRCMP(2, "loc") == 0)
    {
        doutBit = IEC_SGPIO_DOUT_BIT_LOC;
    }
    else if (CLI_PARAM_STRCMP(2, "err") == 0)
    {
        doutBit = IEC_SGPIO_DOUT_BIT_ERR;
    }
    else
    {
        return CLI_STATUS_INVALID_PARAMETER;
    }

    if (CLI_PARAM_STRCMP(3, "on") == 0)
    {
        iecSgpioSetDoutSingleLogicalPhy(logicalPhyNum,
                                  IEC_SGPIO_DOUT_HIGH,
                                  doutBit,
                                  FALSE);
    }
    else if (CLI_PARAM_STRCMP(3, "off") == 0)
    {
        iecSgpioSetDoutSingleLogicalPhy(logicalPhyNum,
                                  IEC_SGPIO_DOUT_LOW,
                                  doutBit,
                                  FALSE);
    }
    else if (CLI_PARAM_STRCMP(3, "blink") == 0)
    {
        iecSgpioSetDoutSingleLogicalPhy(logicalPhyNum,
                                  IEC_SGPIO_DOUT_BLINK,
                                  doutBit,
                                  FALSE);
    }
    else
    {
        return CLI_STATUS_INVALID_PARAMETER;
    }

    return CLI_STATUS_SUCCESS;
}

/**
 *
 * @Name:   iecCliGPIO()
 *
 * @Description: This command shows and sets GPIO registers.
 *
 * @param PtrSessionInfo - Pointer to the CLI session information structure.
 *
 * @return result of command execution.
 *
 *****************************************************************************/

CLI_STATUS iecCliGPIO(PTR_CLI_SESSION_INFO PtrSessionInfo)
{
    if (PtrSessionInfo->TokenInCmdRcd == 1)
    {
        HALI_GPIO_PIN gpioPin;
        U8 gpioValue;
        IEC_GPIO_DIRECTION gpioMode;

        CLI_PRINTF("\r\nGPIO      MODE      Value\r\n");

        for (gpioPin = HALI_GPIO_0; gpioPin < HALI_GPIO_NUMBER; gpioPin++)
        {
            gpioMode = iecGpioGetDirection(gpioPin);

            gpioValue = iecGpioGetPinValue(gpioPin);

            CLI_PRINTF("%-4d      %-10s%-10s\r\n", gpioPin,
                      gpioMode == IEC_GPIO_DIRECTION_INPUT? "Input" : "output",
                      gpioValue == 1? "High": "Low" );
        }

        return CLI_STATUS_SUCCESS;
    }
    else if (PtrSessionInfo->TokenInCmdRcd == 5)
    {
        U32 gpioPin;
        U8 invalidChar;
        S32 retVal;

        retVal = sscanf((const char *)PtrSessionInfo->PtrCmdParams[1], "%d%c",
                            &gpioPin, &invalidChar);
        if ((retVal != 1)
            || (gpioPin > HALI_GPIO_NUMBER))
        {
            return CLI_STATUS_INVALID_PARAMETER;
        }

        if (strcmp((const char *)PtrSessionInfo->PtrCmdParams[2], "set") == 0)
        {
            if (strcmp((const char *)PtrSessionInfo->PtrCmdParams[3],
                "dir") == 0)
            {
                if (strcmp((const char *)PtrSessionInfo->PtrCmdParams[4],
                    "in") == 0)
                {
                    iecGpioSetDirection((HALI_GPIO_PIN)gpioPin,
                        IEC_GPIO_DIRECTION_INPUT);

                    return CLI_STATUS_SUCCESS;
                }
                else if (strcmp((const char *)PtrSessionInfo->PtrCmdParams[4],
                    "out") == 0)
                {
                    iecGpioSetDirection((HALI_GPIO_PIN)gpioPin,
                        IEC_GPIO_DIRECTION_OUTPUT);

                    return CLI_STATUS_SUCCESS;
                }

                return CLI_STATUS_INVALID_PARAMETER;
            }
            else if (strcmp((const char *)PtrSessionInfo->PtrCmdParams[3],
                "val") == 0)
            {
                /* Check whether the GPIO has been set as output. */
                if (iecGpioGetDirection((HALI_GPIO_PIN)gpioPin)
                    != IEC_GPIO_DIRECTION_OUTPUT)
                {
                    CLI_PRINTF("Warning: GPIO %d has not been set as output!\r\n",
                        gpioPin);

                     return CLI_STATUS_INVALID_PARAMETER;
                }
            
                if (strcmp((const char *)PtrSessionInfo->PtrCmdParams[4],
                    "0") == 0)
                {
                    iecGpioSetPinOutput((HALI_GPIO_PIN)gpioPin,
                        HALI_GPIO_BIT_CLEAR);

                    return CLI_STATUS_SUCCESS;
                }
                else if (strcmp((const char *)PtrSessionInfo->PtrCmdParams[4],
                    "1") == 0)
                {
                    iecGpioSetPinOutput((HALI_GPIO_PIN)gpioPin,
                        HALI_GPIO_BIT_SET);

                    return CLI_STATUS_SUCCESS;
                }
                else if (strcmp((const char *)PtrSessionInfo->PtrCmdParams[4],
                    "toggle") == 0)
                {
                    iecGpioTogglePinOutput((HALI_GPIO_PIN)gpioPin);

                    return CLI_STATUS_SUCCESS;
                }

               return CLI_STATUS_INVALID_PARAMETER;

            }
        }

        
        return CLI_STATUS_INVALID_PARAMETER;
    }
    
    return CLI_STATUS_INVALID_PARAM_NUM;
}

/**
 *
 * @Name:   iecCliGPIO()
 *
 * @Description: This command shows and sets address.
 *
 * @param PtrSessionInfo - Pointer to the CLI session information structure.
 *
 * @return result of command execution.
 *
 *****************************************************************************/

CLI_STATUS iecCliSasAddr(PTR_CLI_SESSION_INFO PtrSessionInfo)
{
    SAS_ADDRESS sasAddr;
    HALI_PHY_INFO phyInfo;
    U32 phyIndex;
    HALI_PHY_INFO_STATUS retVal;

    if (PtrSessionInfo->TokenInCmdRcd == 1)
    {
        PTR_SAS_ADDRESS ptrSASAddr = oemGetSasAddressPage(HALI_MFG_USC_SAS_ADDRESS_ID);
        CLI_PRINTF("\r\nActive SAS Address:      %08X-%08lX\r\n",
                        ptrSASAddr->Word.High,
                        ptrSASAddr->Word.Low);


        iecGetSasAddrFromFlash(&sasAddr, HALI_FLASH_OEM_1);
        CLI_PRINTF("Flash OEM 1 SAS Address: %08X-%08lX\r\n",
                            sasAddr.Word.High,
                            sasAddr.Word.Low);

        iecGetSasAddrFromMfg(&sasAddr);
        CLI_PRINTF("MFG SAS Address:         %08X-%08lX\r\n",
                            sasAddr.Word.High,
                            sasAddr.Word.Low);

        for (phyIndex = HALI_EXP_NUM_PHYS; phyIndex < HALI_EXP_NUM_PHYS + 3; phyIndex++)
        {
            retVal = haliGetPhyInformation(&phyInfo, phyIndex);
            if (retVal == HALI_PHY_INFO_SUCCESS)
            {
                if (phyInfo.IsSMPInitiatorAttached)
                {
                    CLI_PRINTF("SMP Initiator Address:   %08X-%08lX\r\n",
                        phyInfo.AttachedSasAddr.Word.High,
                        phyInfo.AttachedSasAddr.Word.Low);
                }

                if (phyInfo.IsSSPInitiatorAttached)
                {
                    CLI_PRINTF("SSP Initiator Address:   %08X-%08lX\r\n",
                        phyInfo.AttachedSasAddr.Word.High,
                        phyInfo.AttachedSasAddr.Word.Low);
                }   
                
                if (phyInfo.IsSMPTgtAttached)
                {
                    CLI_PRINTF("SMP Target Address:      %08X-%08lX\r\n",
                        phyInfo.AttachedSasAddr.Word.High,
                        phyInfo.AttachedSasAddr.Word.Low);
                }
                
                if (phyInfo.IsSSPTgtAttached)
                {
                    CLI_PRINTF("SSP Target Address:      %08X-%08lX\r\n",
                        phyInfo.AttachedSasAddr.Word.High,
                        phyInfo.AttachedSasAddr.Word.Low);
                }                
            }
        }

        return CLI_STATUS_SUCCESS;
    }
    else if (PtrSessionInfo->TokenInCmdRcd == 3)
    {
        U8 invalidChar;
        S32 retVal;
        BOOL setResult;
        
        retVal = sscanf((const char *)PtrSessionInfo->PtrCmdParams[1], "%lx%c",
                            &sasAddr.Word.High, &invalidChar);
        if (retVal == 1)
        {
            retVal = sscanf((const char *)PtrSessionInfo->PtrCmdParams[2], "%lx%c",
                            &sasAddr.Word.Low, &invalidChar);
            if (retVal == 1)
            {
                            
                setResult = iecSetSasAddrToFlash(&sasAddr,
                                    HALI_FLASH_OEM_1);

                if (setResult == TRUE)
                {
                    return CLI_STATUS_SUCCESS;
                }
                else
                {
                    return CLI_STATUS_FAILED;
                }
            }
        }
    }

    return CLI_STATUS_INVALID_PARAMETER;
}

/**
 *
 * @Name:   iecCliLog()
 *
 * @Description: This command adds iec logs.
 *
 * @param PtrSessionInfo - Pointer to the CLI session information structure.
 *
 * @return result of command execution.
 *
 *****************************************************************************/

CLI_STATUS iecCliLog(PTR_CLI_SESSION_INFO PtrSessionInfo)
{
    if (PtrSessionInfo->TokenInCmdRcd == 2)
    {
        if (strlen((const char*)PtrSessionInfo->PtrCmdParams[1]) 
            < HALI_LOG_ENTRY_STRING_ARG_LEN)
        {
            iecAddLog((char*)PtrSessionInfo->PtrCmdParams[1],
                HALI_LOG_CLASS_INFO,
                IEC_LOG_CATEGORY_INIT);

             return CLI_STATUS_SUCCESS;
        }
        else
        {
            return CLI_STATUS_INVALID_PARAMETER;
        }
    }
    
    return CLI_STATUS_INVALID_PARAM_NUM;    
}


/**
 *
 * @Name:   iecCliIstwi()
 *
 * @Description: This command test Istwi Interface
 *
 * @param PtrSessionInfo - Pointer to the CLI session information structure.
 *
 * @return result of command execution.
 *
 *****************************************************************************/
CLI_STATUS iecCliIstwi(PTR_CLI_SESSION_INFO PtrSessionInfo)
{
    U32 channel;
    U8 invalidChar;
    S32 retVal;
    U8 addr7bits;
    HALI_ISTWI_STATUS istwiStauts;
    HALI_ISTWI_ADDRESS istwiAddress;
    U8 ptrBlock[1] = {0};
    U32 count = 1;
    U32 timeout = 5000;
    U32 hwTimeout = 2000000 / haliOsGetMicrosecPerTick();

    if (strcmp((const char *)PtrSessionInfo->PtrCmdParams[1], "scan") == 0)
    {
        retVal = sscanf((const char *)PtrSessionInfo->PtrCmdParams[2], "%d%c",
                        &channel, &invalidChar);

        if (retVal == 1)
        {
            CLI_PRINTF("\r\n");
            
            for (addr7bits = 0; addr7bits < 0x80; addr7bits++)
            {
                istwiAddress.Addr1.Bits.Address = addr7bits;
                
                istwiStauts = haliIstwiRead((HALI_ISTWI_CHANNEL)channel,
                                            istwiAddress,
                                            ptrBlock,
                                            count,
                                            timeout,
                                            hwTimeout);
                if (istwiStauts != HALI_ISTWI_ERROR_NAK_RX_DURING_ADDR_PHASE
                    && istwiStauts != HALI_ISTWI_ERROR_LOST_ARB)
                {
                    CLI_PRINTF("ACKed addr 7bits: %X, 8bits: %X, rv: %X\r\n",
                        addr7bits,
                        (addr7bits << 1),
                        istwiStauts);
                }
                    
                haliOsThreadSleep(100);
            }

            return CLI_STATUS_SUCCESS;
        }
    }

    return CLI_STATUS_INVALID_PARAMETER;
}

#ifndef PRODUCTION_RELEASE
CLI_STATUS iecCliTest(PTR_CLI_SESSION_INFO PtrSessionInfo)
{
    U32 phy;
    U8 invalidChar;
    IEC_FWERR retVal;
    U8 powerMode;

    if (CLI_ARGC != 2)
        return CLI_STATUS_INVALID_PARAM_NUM;

    CLI_PARAM_PARSE_U32(1, &phy, &invalidChar);

    retVal = iecAtaCheckPowerMode(haliPhyRemapLogicalToPhysical(phy),
                    &powerMode);
    if (retVal == IEC_SUCCESS)
    {
        CLI_PRINTF("\r\n Power mode: %x", powerMode);

        return CLI_STATUS_SUCCESS;
    }
    else
    {
        return CLI_STATUS_FAILED;
    }
}
#endif

/**
 *
 * @Name:	iecCliTemperatureSensor()
 *
 * @Description: This command shows the info of temperature sensors.
 *              We may set the index the same with their SES element index.
 *
 * @param PtrSessionInfo - Pointer to the CLI session information structure.
 *
 * @return result of command execution.
 *
 *****************************************************************************/

CLI_STATUS iecCliTemperatureSensor(PTR_CLI_SESSION_INFO PtrSessionInfo)
{
    S32 socTemp;

    if (CLI_ARGC == 1)
    {
        socTemp = iecGetExpanderTemperature();

        CLI_PRINTF("\r\nSOC: %d'C", socTemp);
    }
    else
    {
        return CLI_STATUS_INVALID_PARAM_NUM;
    }
   
    return CLI_STATUS_SUCCESS;
}

/**
 *
 * @Name:   iecCliGetFWRegionInfo()
 *
 * @Description: helper function to print fw region info.
 *
 * @param PtrSessionInfo - Pointer to the CLI session information structure.
 *
 * @param RegionID - one of region id: 0, 1, 2, 3
 *
 * @return result of command execution.
 *
 *****************************************************************************/

static CLI_STATUS iecCliGetFWRegionInfo(PTR_CLI_SESSION_INFO PtrSessionInfo,
                            HALI_FLASH_REGION_TYPE RegionID)
{
    HALI_FLASH_STATUS status;
    CLI_STATUS retVal = CLI_STATUS_SUCCESS; 
    extern HALI_INIT_BOOT_INFO  gFwBootInfo;
    U32 runningRegion = gFwBootInfo.ActiveFwImage;
    FW_HEADER header;
    U32 contentSum;
    U32 regionSize;
    
    haliFlashRegionRead((PU8)&header, RegionID, 0, sizeof(FW_HEADER));
    regionSize = haliFlashGetRegionSize(RegionID);

    CLI_PRINTF("%X\t", RegionID);

    switch (RegionID)
    {
        case HALI_FLASH_BOOT_LOADER:
            CLI_PRINTF("Boot\t");
            break;
        
        case HALI_FLASH_FIRMWARE_COPY_1:
            CLI_PRINTF("Copy 1\t");
            break;
        
        case HALI_FLASH_FIRMWARE_COPY_2:
            CLI_PRINTF("Copy 2\t");
            break;
        
        case HALI_FLASH_MANUFACTURING:
            CLI_PRINTF("MFG\t");
            break;
        
        default:
            CLI_PRINTF("N/A\t");
            break;
    }

    if (RegionID == HALI_FLASH_BOOT_LOADER
        || RegionID == HALI_FLASH_FIRMWARE_COPY_1
        || RegionID == HALI_FLASH_FIRMWARE_COPY_2)
    {
        FW_HEADER header;
        
        if (runningRegion == RegionID)
            CLI_PRINTF("Y\t");
        else
            CLI_PRINTF("N\t");

        CLI_PRINTF("\tY\t");

        status = haliFlashRegionRead((PU8)&header, RegionID, 0, sizeof(FW_HEADER));

        if (status == HALI_FLASH_SUCCESS)
        {
            CLI_PRINTF("\t0x%X\t", header.FwImageSize);

            if (regionSize > header.FwImageSize)
                contentSum = iecCalFlashRegionU32ContentSum(RegionID, header.FwImageSize);
            else
                contentSum = 1;

            CLI_PRINTF("0x%X\t", contentSum);

            if (contentSum == 0)
                CLI_PRINTF("Y\r\n");
            else
                CLI_PRINTF("N\r\n");
        }
        else
        {
            retVal = CLI_STATUS_FLASH_ERROR;
        }
    }
    else if (RegionID == HALI_FLASH_MANUFACTURING)
    {
        contentSum = iecCalFlashRegionU32ContentSum(RegionID, 13540);

        /* Running */
        CLI_PRINTF("Y\t");

        /* Active after reset */
        CLI_PRINTF("\tY\t");

        /* Length */
        CLI_PRINTF("\t0x%X\t", 0);
        
        CLI_PRINTF("0x%X\t", contentSum);

        if (haliIsMfgRegionValid())
            CLI_PRINTF("Y\r\n");
        else
            CLI_PRINTF("N\r\n");
    }
    else
    {
        retVal = CLI_STATUS_INVALID_PARAMETER;
    }

    return retVal;
}

/**
 *
 * @Name:   iecDisplayFlashRegionInfo()
 *
 * @Description: helper function to print fw region info.
 *
 * @param PtrSessionInfo - Pointer to the CLI session information structure.
 *
 * @return result of command execution.
 *
 *****************************************************************************/

static void iecDisplayFlashRegionInfo(PTR_CLI_SESSION_INFO PtrSessionInfo)
{
    HALI_FLASH_REGION_TYPE RegionID;

    CLI_PRINTF("\r\n");
    CLI_PRINTF("Region  Name    RunningNow      ActiveAftetReset Length ContentSum  Valid\r\n");
    CLI_PRINTF(IEC_CLI_PRINT_HEADER);
    CLI_PRINTF("\r\n");
    
    for (RegionID = HALI_FLASH_BOOT_LOADER; RegionID < HALI_FLASH_CONFIG_1; RegionID++)
    {
        iecCliGetFWRegionInfo(PtrSessionInfo, RegionID);
    }
}

/**
 *
 * @Name:   iecCliFlashRegionDiff()
 *
 * @Description: This command diffs the contents of two flash region
 *
 * @param PtrSessionInfo - Pointer to the CLI session information structure.
 * 
 * @param FirstRegion - first flash region id
 *
 * @param SecondRegion - second flash region id
 *
 * @param Length - the length to read from both two regions
 *
 * @return result of command execution.
 *
 *****************************************************************************/

CLI_STATUS iecCliFlashRegionDiff(PTR_CLI_SESSION_INFO PtrSessionInfo,
                                        HALI_FLASH_REGION_TYPE FirstRegion,
                                        HALI_FLASH_REGION_TYPE SecondRegion,
                                        U32 Length)
{
    U32 index;
    U32 offset;
    U32 blockSize;
    PU8 ptrRegion1;
    PU8 ptrRegion2;
    CLI_STATUS status = CLI_STATUS_SUCCESS;
    
    ptrRegion1 = haliCalloc(HALI_MEMORY_ID_IMEM, 1024, 1);
    ptrRegion2 = haliCalloc(HALI_MEMORY_ID_IMEM, 1024, 1);
    
    for (offset = 0; offset < Length; offset+=blockSize)
    {
        if ((Length - offset) > 1024)
            blockSize = 1024;
        else
            /* The last block to read */
            blockSize = Length - offset;

        /* avoid cpu hogging */
        haliOsThreadSleep(2);
        
        if (haliFlashRegionRead(ptrRegion1,
                HALI_FLASH_FIRMWARE_COPY_1,
                offset, 
                blockSize) == HALI_FLASH_SUCCESS
                &&
            haliFlashRegionRead(ptrRegion2,
                HALI_FLASH_FIRMWARE_COPY_2,
                offset, 
                blockSize) == HALI_FLASH_SUCCESS)
        {
            for (index = 0; index<blockSize; index++)
            {
                if (ptrRegion1[index] != ptrRegion2[index])
                {
                    CLI_PRINTF("diff-offset0x%X-0x%X-0x%X\r\n",
                        offset+index, ptrRegion1[index], ptrRegion2[index]);
                }
            }
        }
        else
        {
            CLI_PRINTF("read error\r\n");

            status =  CLI_STATUS_FLASH_ERROR;
        }
    }

    haliFree(ptrRegion1);
    haliFree(ptrRegion2);

    return status;
}

/**
 *
 * @Name:   iecCliFwInfo()
 *
 * @Description: This command shows the info of flash region 0 to 3.
 *
 * @param PtrSessionInfo - Pointer to the CLI session information structure.
 *
 * @return result of command execution.
 *
 *****************************************************************************/

CLI_STATUS iecCliFwInfo(PTR_CLI_SESSION_INFO PtrSessionInfo)
{
    HALI_FLASH_REGION_TYPE regionId, secRegionId;
    U32 length;
    U8 invalid;
    CLI_STATUS status = CLI_STATUS_SUCCESS;

    if (CLI_ARGC == 1)
    { 
        iecDisplayFlashRegionInfo(PtrSessionInfo);

    }
    else if (CLI_ARGC == 5)
    {
        if (CLI_PARAM_STRCMP(1, "cmp") == 0)
        {
            CLI_PARAM_PARSE_U32(2, (U32*)&regionId, &invalid);
            CLI_PARAM_PARSE_U32(3, (U32*)&secRegionId, &invalid);
            CLI_PARAM_PARSE_U32(4, &length, &invalid);

            if (regionId > HALI_FLASH_MANUFACTURING
                || secRegionId > HALI_FLASH_MANUFACTURING)
               return CLI_STATUS_INVALID_PARAMETER;

            //todo: check length

            status = iecCliFlashRegionDiff(PtrSessionInfo,
                                        regionId, 
                                        secRegionId, 
                                        length);
               
        }
    }

    return status;
}

/**
 *
 * @Name:	iecCliDisplaySmartData()
 *
 * @Description: This command display smart data by attributes
 *
 * @param PtrSessionInfo - Pointer to the CLI session information structure.
 *
 * @param PtrData - pointer to the SMART DATA
 *
 *****************************************************************************/

static void iecCliDisplaySmartData(PTR_CLI_SESSION_INFO PtrSessionInfo,
                                       PU8 PtrData)
{
    U32 index;

    PTR_ATA_SMART_DATA ptrSmartData = (PTR_ATA_SMART_DATA)PtrData;

    PTR_ATA_SMART_ATTRS ptrAttrs = ptrSmartData->VendorAttributes; 
    
    CLI_PRINTF("\r\nID#     FLAG     VALUE WORST RAW_VALUE[0]   THRESHOLD\r\n");
    
    for(index = 0; index < 30; index++) 
    {
        if (ptrAttrs->AttrId == IEC_SMART_INVLAID_ATTR_ID)
            break;
    
        CLI_PRINTF("%3d     0x%04X     %3d  %3d   0x%02X%02X%02X%02X%02X%02X   %02X\r\n",
           ptrAttrs->AttrId,
           ptrAttrs->Flags, 
           ptrAttrs->CurrentValue, 
           ptrAttrs->Worst,
           ptrAttrs->Raw[5],
           ptrAttrs->Raw[4],
           ptrAttrs->Raw[3],
           ptrAttrs->Raw[2],
           ptrAttrs->Raw[1],
           ptrAttrs->Raw[0],
           ptrAttrs->Reserved);
        
        ptrAttrs++;
    }    

    CLI_PRINTF("Short Test Recommended polling time in minutes: %d\r\n", 
        ptrSmartData->ShortTestCompletionTime);

    CLI_PRINTF("Extended Test Recommended polling time in minutes: %d-%d\r\n", 
        ptrSmartData->ExtendTestCompletionTimeB,
        ptrSmartData->ExtendTestCompletionTimeW);
}


									   /**
 *
 * @Name:	iecCliDisplayWarrantyThreshold()
 *
 * @Description: This command display smart data by attributes
 *
 * @param PtrSessionInfo - Pointer to the CLI session information structure.
 *
 * @param PtrData - pointer to the SMART DATA
 *
 *****************************************************************************/
static void iecCliDisplayWarrantyThreshold(PTR_CLI_SESSION_INFO PtrSessionInfo,
                                                   PU8 PtrData)
{
	
    U32 index;

    PTR_ATA_SMART_DATA ptrSmartData = (PTR_ATA_SMART_DATA)PtrData;

    PTR_ATA_SMART_ATTRS_WARRANTY ptrAttrs = (PTR_ATA_SMART_ATTRS_WARRANTY)ptrSmartData->VendorAttributes; 
	CLI_PRINTF("%04X\r\n", ptrSmartData->RevNumber);
    
    CLI_PRINTF("\r\nID#    FLAG     Reserved_Raw[0]\r\n");
    
    for(index = 0; index < 30; index++) 
    {
        if (ptrAttrs->AttrId == IEC_SMART_INVLAID_ATTR_ID)
            break;
    
        CLI_PRINTF("%3d    %3d    0x%02X%02X%02X%02X%02X%02X\r\n",
           ptrAttrs->AttrId,
           ptrAttrs->threshold, 
           ptrAttrs->Reserverd[5],
           ptrAttrs->Reserverd[4],
           ptrAttrs->Reserverd[3],
           ptrAttrs->Reserverd[2],
           ptrAttrs->Reserverd[1],
           ptrAttrs->Reserverd[0]);
        
        ptrAttrs++;
    }
	    CLI_PRINTF("the checksum value for micron: %d\r\n", 
	              ptrSmartData->chksum);
	
}


/**
 *
 * @Name:	iecCliSmartReadData()
 *
 * @Description: This command issues a SMART READ DATA to a SATA Drive and 
 *      print the response.
 *
 * @param PtrSessionInfo - Pointer to the CLI session information structure.
 *
 * @return result of command execution.
 *
 *****************************************************************************/

CLI_STATUS iecCliSmartReadData(PTR_CLI_SESSION_INFO PtrSessionInfo)
{
    U32 logicalPhyID, physicalPhyID;
    U8 invalid;
    HALI_STPI_CMD_DATA_RESPONSE cmdRsp;
    BOOL retVal;
    PU8 ptrSmartData;
    HALI_PHY_INFO phyInfo;
    CLI_STATUS status;

    if (CLI_ARGC != 2)
        return CLI_STATUS_INVALID_PARAM_NUM;

    CLI_PARAM_PARSE_U32(1, &logicalPhyID, &invalid);

    physicalPhyID = haliPhyRemapLogicalToPhysical(logicalPhyID);

    haliGetPhyInformation(&phyInfo, physicalPhyID);

    if (phyInfo.IsSATATgtAttached)
    {
        ptrSmartData = calloc(1, ATA_SMART_DATA_LEN);

        CLI_PRINTF("\r\n");

        retVal = iecAtaSmart(physicalPhyID, 
                            SMART_READ_DATA,
                            ATA_LBA_SMART_READ_DATA,
                            IEC_SMART_READ_DATA_COUNT,
                            &cmdRsp, 
                            ptrSmartData, 
                            ATA_SMART_DATA_LEN);

        if (retVal == IEC_SUCCESS)
        {
            iecCliDisplaySmartData(PtrSessionInfo, ptrSmartData);

            status = CLI_STATUS_SUCCESS;
        }
        else
        {
            status = CLI_STATUS_FAILED;
        }

        free(ptrSmartData);
    }
    else
    {
        CLI_PRINTF("No SATA Device attached!\r\n");

        status = CLI_STATUS_FAILED;
    }
    
    return status;
}

/**
 *
 * @Name:	iecCliSmartReadWaTh()
 *
 * @Description: This command issues a SMART READ WARRANTY THRESHOLDS to a SATA Drive and 
 *      print the response.
 *
 * @param PtrSessionInfo - Pointer to the CLI session information structure.
 *
 * @return result of command execution.
 *
 *****************************************************************************/

CLI_STATUS iecCliSmartReadWaTh(PTR_CLI_SESSION_INFO PtrSessionInfo)
{
    U32 logicalPhyID, physicalPhyID;
    U8 invalid;
    HALI_STPI_CMD_DATA_RESPONSE cmdRsp;
    BOOL retVal;
    PU8 ptrSmartData;
    HALI_PHY_INFO phyInfo;
    CLI_STATUS status;

    if (CLI_ARGC != 2)
        return CLI_STATUS_INVALID_PARAM_NUM;
	//fill the argv[0] with logicalphyid  

    CLI_PARAM_PARSE_U32(1, &logicalPhyID, &invalid);

    physicalPhyID = haliPhyRemapLogicalToPhysical(logicalPhyID);

    haliGetPhyInformation(&phyInfo, physicalPhyID);

    if (phyInfo.IsSATATgtAttached)
    {
        ptrSmartData = calloc(1, ATA_SMART_DATA_LEN);

        CLI_PRINTF("\r\n");

        retVal = iecAtaSmart(physicalPhyID, 
                            SMART_READ_WARRANTY_THRESHOLD,
                            ATA_LBA_SMART_READ_DATA,
                            IEC_SMART_READ_DATA_COUNT,
                            &cmdRsp, 
                            ptrSmartData, 
                            ATA_SMART_DATA_LEN);

        if (retVal == IEC_SUCCESS)
        {
            iecCliDisplayWarrantyThreshold(PtrSessionInfo, ptrSmartData);
				printf("verify count for checksum: %d\r\n", 
	            iecCalU8Checksum(ptrSmartData, 511));//verify checksum

            status = CLI_STATUS_SUCCESS;
        }
        else
        {
            status = CLI_STATUS_FAILED;
        }

        free(ptrSmartData);
    }
    else
    {
        CLI_PRINTF("No SATA Device attached!\r\n");

        status = CLI_STATUS_FAILED;
    }
   
    return status;
}


/**
 * @Name:   iecCliRawDataPrintf()
 *
 * @Description: This function print input raw data in hex.
 *
 * @param String - msg to printf before data
 * @param PtrData   - point to data to be printf
 * @param DataSize  - Number of bytes to be printf
 *
 *****************************************************************************/

void iecCliRawDataPrintf(PTR_CLI_SESSION_INFO PtrSessionInfo,
                            char *String,
							void const * PtrData, 
							U16 DataSize)
{
	U16 idx;
	U8  *ptr = (PU8)PtrData;

	if (ptr != NULL && DataSize > 0)
	{
		if (String)
		{
			printf("%s\r\n", String);
		}

        CLI_PRINTF("00000000: ");
        
		for (idx = 0; idx < DataSize; idx++)
		{
			/* print 16 bytes each line. */
			if (idx && (idx % 16 == 0))
			{
				CLI_PRINTF("\r\n");
                CLI_PRINTF("%08X: ", idx);
			}

			CLI_PRINTF("%02X ", ptr[idx]);
		}
	}
	else
	{
		CLI_PRINTF("No data to print!\r\n");
	}
	CLI_PRINTF("\r\n");
}


/**
 *
 * @Name:	iecCliEnclosure()
 *
 * @Description: This command shows info of IEC Enclosure Mangement module.
 *
 * @param PtrSessionInfo - Pointer to the CLI session information structure.
 *
 * @return result of command execution.
 *
 *****************************************************************************/

CLI_STATUS iecCliEnclosure(PTR_CLI_SESSION_INFO PtrSessionInfo)
{
    if (CLI_ARGC == 2)
    {
        if (CLI_PARAM_STRCMP(1, "show") == 0)
        {
            if (iecLockSavedEnclData(HALI_OS_WAIT_FOREVER) == HALI_OS_SUCCESS)
            {
                PTR_IEC_ENCL_DATA ptrData = iecGetSavedEnclDataPtr();

                CLI_PRINTF("\r\n");

                iecCliRawDataPrintf(PtrSessionInfo,
                                "ENCL Data",
                                ptrData,
                                sizeof(IEC_ENCL_DATA));

                iecUnLockSavedEnclData();
            }
        }
        else
        {
            return CLI_STATUS_INVALID_PARAMETER;
        }
    }
    else
    {
        return CLI_STATUS_INVALID_PARAM_NUM;
    }
    
    return CLI_STATUS_SUCCESS;
}

/**
 *
 * @Name:	iecCliAtaDevTemperature()
 *
 * @Description: This command show cached ATA device temperature or retrieving
 *              the temperature immediately.
 *
 * @param PtrSessionInfo - Pointer to the CLI session information structure.
 *
 * @return result of command execution.
 *
 *****************************************************************************/

CLI_STATUS iecCliAtaDevTemperature(PTR_CLI_SESSION_INFO PtrSessionInfo)
{
    U32 driveId;
    U8 invalidChar;
    U8 phyId;
    HALI_PHY_INFO phyInfo;
    CLI_STATUS status = CLI_STATUS_SUCCESS;
    PU8 ptrBuffer;

    if (CLI_ARGC == 2)
    {
        CLI_PARAM_PARSE_U32(1, &driveId, &invalidChar);
        
        phyId = arbokDriveIdToPhysicalPhyId(driveId);

        haliGetPhyInformation(&phyInfo, phyId);

        if (!phyInfo.IsSATATgtAttached)
        {
            CLI_PRINTF("\r\nNo SATA Drive Attached!\r\n");

            return CLI_STATUS_FAILED;
        }

        ptrBuffer = malloc(IEC_ATA_TEMPERATUER_GET_BUFFER_SZ);
        #ifdef IEC_ATA_SMART_ATTR
        CLI_PRINTF("\r\nUsing Smart Attributes: %d\r\n",
                    iecAtaGetTempBySmartAttr(phyId, ptrBuffer));
        #endif
        if (iecAtaIsSctSupported(phyId))
        {

            CLI_PRINTF("Using SCT SMART: %d\r\n",
                iecAtaGetTempBySctSmart(phyId, ptrBuffer));
       #ifdef IEC_ATA_SUPPORT_GPL
            CLI_PRINTF("Using SCT GPL: %d\r\n",
                iecAtaGetTempBySctGpl(phyId, ptrBuffer));
	   #endif
        }
        else
        {
            CLI_PRINTF("SCT is not supported\r\n");
        }
       #ifdef IEC_ATA_SUPPORT_STATUS
        if (iecAtaIsTempStatSupported(phyId))
        {
            CLI_PRINTF("Using Temperature Stat: %d\r\n",
                iecAtaGetTempByTempStat(phyId, ptrBuffer));
        }
        else
        {
            CLI_PRINTF("Temperature Stat is not supported\r\n");
        }
		#endif
        free(ptrBuffer);
    }
    else
    {
        status = CLI_STATUS_INVALID_PARAM_NUM;
    }

    if (status == CLI_STATUS_SUCCESS)
    {
        CLI_PRINTF(IEC_CLI_PRINT_HEADER);
        CLI_PRINTF("\r\n0xFF: failed to get temperature\r\n");
        CLI_PRINTF("0x80: invalid temperature returned by device\r\n");
    }
    
    return status;
}



/**
 * @Name:   iecCliInit()
 *
 * @Description: This function registers IEC specific CLI commands.
 *
 * @param PtrCliCmdList - Pointer to the CLI Command List
 *               structure.
 *
 * @note This function should be called at cliCoreInit().
 *
 *****************************************************************************/
 
void iecCliInit( PTR_CLI_CMD_LIST PtrCliCmdList )
{

    U16 cmdCount;
	U16 inx;
	

	/* No. of entries in sPtrIecCliCmdList */
	cmdCount = sizeof(sPtrIecCliCmdList)/sizeof(sPtrIecCliCmdList[0]);

	/* Traverse through the list and register all commands with CLI core */
	for( inx = 0; inx < cmdCount; inx++ )
	{
		cliRegisterCommand(sPtrIecCliCmdList[inx]->PtrCmdName,
							sPtrIecCliCmdList[inx]->PtrOneLineHelp,
							sPtrIecCliCmdList[inx]->PtrToFunCall,
							PtrCliCmdList);
	}
	
}

/**
 * @Name:   __inline U8  cliArrowHandle(U8 InputChar)
 *
 * @Description: This function will detect arrow key pressed and return corresponding
 *               key value.
 *
 * @param  InputChar denote getchar every one time
 *
 * @return  UP_KEY  value 2
            DOWN_KEY  value 4;
 *
 *****************************************************************************/ 
__inline U8  iecCliArrowHandle(U8 InputChar)
{     
      static U8   sum = 0;
      if(InputChar == 0x00)
        return 0;   
      switch(InputChar)
      {
            case 0X5b:  //represent for '['
                sum += 1;
                break;
                
            case 0X41:  //represent for 'A'
                sum += 1;
                break;
             case 0X42:
                sum += 3; //avoid press single 'B'
                break;
            /*initial press arrow key again*/
            default:
                sum = 0;
                break;
        }
        return sum;
}

/**
 * @Name:   iecCliSearchCommand()
 *
 * @Description: This function search the command set by the partial string inputed
 *               by user.
 *
 * @param  ptrName - pointer to  user input string
 *
 * @param len - the length of user input string
 *
 * @param pptrResult - pointer to pointer to the last command in the set that is matched.
 *
 * @return count of commands that are matched.
 *
 * @author Xie.Qian-jun 20180119
 *****************************************************************************/
U8 iecCliSearchCommand(const char *ptrName, 
                                          U8 len, 
                                          char**pptrResult, 
                                          CLI_CMD_LIST sCliCmdList)
{
    U8 count = 0;

    PTR_CLI_CMD_NODE PtrCmdNode  = sCliCmdList.PtrCliCmdListHead;
    *pptrResult = NULL;
    
    for (; PtrCmdNode; PtrCmdNode = PtrCmdNode->PtrNext)
    {
        if( strncmp((char *)(PtrCmdNode->Command),
                    ptrName,
                    len) == 0 )
        {
            count++;
            *pptrResult = (char *)PtrCmdNode->Command;
        }
    
    }

    return count;
}


/** @} */

