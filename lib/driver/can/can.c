/**
 * @file    can.c
 * @brief   FlexCAN Driver Implementation for S32K144
 * @details Implementation of CAN driver functions
 * 
 * @author  PhucPH32
 * @date    30/11/2025
 * @version 1.0
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "can.h"
#include "../nvic/nvic.h"
#include "../pcc/pcc.h"
#include <stddef.h>

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define MSG_BUF_SIZE  4  /* Message Buffer Size: 4 words (CS, ID, DATA0, DATA1) */
/*******************************************************************************
 * Private Definitions
 ******************************************************************************/

/** @brief Timeout for entering/exiting freeze mode */
#define CAN_FREEZE_TIMEOUT      (10000U)

/** @brief Default CAN clock frequency (48 MHz from FIRC DIV2) */
//#define CAN_DEFAULT_CLK_FREQ    (24000000U)

/** @brief Default CAN clock frequency (40 MHz from Bus clock) */
#define CAN_DEFAULT_CLK_FREQ    (40000000U)
// TO DO Change this
/*******************************************************************************
 * Private Variables
 ******************************************************************************/

/** @brief CAN base address array */
static CAN_Type * const s_canBases[CAN_INSTANCE_COUNT] = {CAN0, CAN1, CAN2};

/** @brief CAN initialized flag */
static bool s_canInitialized[CAN_INSTANCE_COUNT] = {false, false, false};

/** @brief Unified callback storage */
static can_callback_t s_canCallbacks[CAN_INSTANCE_COUNT] = {NULL, NULL, NULL};

/*******************************************************************************
 * Private Function Prototypes
 ******************************************************************************/

static uint8_t CAN_GetInstanceIndex(CAN_Type *base);
static status_t CAN_EnterFreezeMode(CAN_Type *base);
static status_t CAN_ExitFreezeMode(CAN_Type *base);
static status_t CAN_SoftReset(CAN_Type *base);
static void CAN_EnableClock(uint8_t instance, can_clk_src_t clockSource);
static void CAN_InitMessageBuffers(CAN_Type *base);
static uint32_t CAN_GetClockFrequency(can_clk_src_t clockSource);

/*******************************************************************************
 * Public Functions
 ******************************************************************************/

/**
 * @brief Initialize CAN module
 */
status_t CAN_Init(const can_config_t *config)
{
    CAN_Type *base;
    status_t status;
    can_timing_config_t timing;
    uint32_t canClockHz;
    
    /* Validate parameters */
    if (config == NULL) {
        return STATUS_INVALID_PARAM;
    }
    
    if (config->instance >= CAN_INSTANCE_COUNT) {
        return STATUS_INVALID_PARAM;
    }
    
    base = s_canBases[config->instance];
    
    /* Enable CAN clock */
    CAN_EnableClock(config->instance, config->clockSource);
    
    /* Get CAN clock frequency */
    canClockHz = CAN_GetClockFrequency(config->clockSource);
    
    /* Enter freeze mode for configuration */
    status = CAN_EnterFreezeMode(base);
    if (status != STATUS_SUCCESS) {
        return STATUS_ERROR;
    }
    
    /* Soft reset */
    status = CAN_SoftReset(base);
    if (status != STATUS_SUCCESS) {
        return STATUS_ERROR;
    }
    
    /* Calculate timing parameters */
    status = CAN_CalculateTiming(canClockHz, config->baudRate, &timing);
    if (status != STATUS_SUCCESS) {
        return STATUS_INVALID_PARAM;
    }
    
    /* Configure CTRL1 register */
    base->CTRL1 = ((uint32_t)timing.preDiv << CAN_CTRL1_PRESDIV_SHIFT) |
                  ((uint32_t)timing.rJumpWidth << CAN_CTRL1_RJW_SHIFT) |
                  ((uint32_t)timing.phaseSeg1 << CAN_CTRL1_PSEG1_SHIFT) |
                  ((uint32_t)timing.phaseSeg2 << CAN_CTRL1_PSEG2_SHIFT) |
                  ((uint32_t)timing.propSeg << CAN_CTRL1_PROPSEG_SHIFT)|
                  ((uint32_t)3<<CAN_CTRL1_SMP_SHIFT);
    
    /* Configure operating mode */
    if (config->mode == CAN_MODE_LOOPBACK) {
        base->CTRL1 |= CAN_CTRL1_LPB_MASK;
    } else if (config->mode == CAN_MODE_LISTEN_ONLY) {
        base->CTRL1 |= CAN_CTRL1_LOM_MASK;
    }
    
    /* Configure self-reception */
    if (!config->enableSelfReception) {
        base->MCR |= CAN_MCR_SRXDIS_MASK;
    }
    
    /* Configure RX FIFO or individual MBs */
    if (config->useRxFifo) {
        base->MCR |= CAN_MCR_RFEN_MASK;
    } else {
        base->MCR &= ~CAN_MCR_RFEN_MASK;
    }
    
    /* Set maximum number of MBs */
    base->MCR = (base->MCR & ~CAN_MCR_MAXMB_MASK) | 
                ((CAN_MB_COUNT - 1U) << CAN_MCR_MAXMB_SHIFT);
    
    /* Initialize all Message Buffers */
    CAN_InitMessageBuffers(base);
    
    /* Initialize global mask to accept all messages */

    base->RXMGMASK = 0x1FFFFFFFUL;
    
    /* Clear error counters */
    base->ECR = 0U;

    /* Exit freeze mode */
    status = CAN_ExitFreezeMode(base);
    if (status != STATUS_SUCCESS) {
        return STATUS_ERROR;
    }
    
    /* Mark as initialized */
    s_canInitialized[config->instance] = true;
    
    return STATUS_SUCCESS;
}

/**
 * @brief Deinitialize CAN module
 */
status_t CAN_Deinit(uint8_t instance)
{
    CAN_Type *base;
    
    /* Validate parameter */
    if (instance >= CAN_INSTANCE_COUNT) {
        return STATUS_INVALID_PARAM;
    }
    
    base = s_canBases[instance];
    
    /* Disable module */
    base->MCR |= CAN_MCR_MDIS_MASK;

    /* Clear initialized flag */
    s_canInitialized[instance] = false;
    
    return STATUS_SUCCESS;
}

/**
 * @brief Send CAN message
 */
status_t CAN_Send(uint8_t instance, uint8_t mbIndex, const can_message_t *message)
{
    CAN_Type *base;
    uint32_t mbOffset;
    uint32_t cs, id;
    
    /* Validate parameters */
    if (instance >= CAN_INSTANCE_COUNT || message == NULL) {
        return STATUS_INVALID_PARAM;
    }
    
    if (!s_canInitialized[instance]) {
        return STATUS_NOT_INITIALIZED;
    }
    
    if (mbIndex < CAN_TX_MB_START ||
        mbIndex >= (CAN_TX_MB_START + CAN_TX_MB_COUNT)) {
        return STATUS_INVALID_PARAM;
    }
    
    if (message->dataLength > CAN_MAX_DATA_LENGTH) {
        return STATUS_INVALID_PARAM;
    }
    
    base = s_canBases[instance];
    mbOffset = mbIndex * MSG_BUF_SIZE;
    
    /* Clear interrupt flag */
    base->IFLAG1 = (1UL << mbIndex);
    
    /* Write data words */
    base->RAMn[mbOffset + 2] = ((uint32_t)message->data[0] << 24) |
                                ((uint32_t)message->data[1] << 16) |
                                ((uint32_t)message->data[2] << 8) |
                                ((uint32_t)message->data[3]);
    
    base->RAMn[mbOffset + 3] = ((uint32_t)message->data[4] << 24) |
                                ((uint32_t)message->data[5] << 16) |
                                ((uint32_t)message->data[6] << 8) |
                                ((uint32_t)message->data[7]);
    
    /* Configure ID word */
    if (message->idType == CAN_ID_STD) {
        id = (message->id << CAN_ID_STD_SHIFT) & CAN_ID_STD_MASK;
    } else {
        id = (message->id << CAN_ID_EXT_SHIFT) & CAN_ID_EXT_MASK;
    }
    base->RAMn[mbOffset + 1] = id;
    
    /* Configure CS word and activate transmission */
    cs = (CAN_CS_CODE_TX_DATA << CAN_CS_CODE_SHIFT) |
         ((uint32_t)message->dataLength << CAN_WMBn_CS_DLC_SHIFT);
    
    if (message->idType == CAN_ID_EXT) {
        cs |= CAN_WMBn_CS_IDE_MASK;
    }
    
    if (message->idType == CAN_ID_STD) {
        cs |= CAN_WMBn_CS_SRR_MASK;  /* SRR=1 for standard ID transmission */
    }
    
    if (message->frameType == CAN_FRAME_REMOTE) {
        cs |= CAN_WMBn_CS_RTR_MASK;
    }
    
    base->RAMn[mbOffset + 0] = cs;  /* Write CS to activate MB */
    
    return STATUS_SUCCESS;
}

/**
 * @brief Send CAN message with timeout
 */
status_t CAN_SendBlocking(uint8_t instance, uint8_t mbIndex, 
                          const can_message_t *message, uint32_t timeoutMs)
{
    CAN_Type *base;
    status_t status;
    uint32_t timeoutCount = 0;
    uint32_t mbMask;
    
    /* Send message */
    status = CAN_Send(instance, mbIndex, message);
    if (status != STATUS_SUCCESS) {
        return status;
    }
    
    base = s_canBases[instance];
    mbMask = (1UL << mbIndex);
    
    /* Wait for transmission complete */
    while (timeoutCount < (timeoutMs * 1000U)) {
        if (base->IFLAG1 & mbMask) {
            /* Clear flag */
            base->IFLAG1 = mbMask;
            return STATUS_SUCCESS;
        }
        timeoutCount++;
    }
    
    return STATUS_TIMEOUT;
}

/**
 * @brief Receive CAN message
 */
status_t CAN_Receive(uint8_t instance, uint8_t mbIndex, can_message_t *message)
{
    CAN_Type *base;
    uint32_t mbOffset;
    uint32_t cs, id;
    uint32_t mbMask;
    uint32_t data0, data1;
    uint32_t dummy;
    
    /* Validate parameters */
    if (instance >= CAN_INSTANCE_COUNT || message == NULL) {
        return STATUS_INVALID_PARAM;
    }
    
    if (!s_canInitialized[instance]) {
        return STATUS_NOT_INITIALIZED;
    }
    
    if (mbIndex < CAN_RX_MB_START || mbIndex >= CAN_MB_COUNT) {
        return STATUS_INVALID_PARAM;
    }
    
    base = s_canBases[instance];
    mbMask = (1UL << mbIndex);
    mbOffset = mbIndex * MSG_BUF_SIZE;
    
    /* Check if message available */
    if ((base->IFLAG1 & mbMask) == 0U) {
        return STATUS_ERROR;
    }
    
    /* Read CS word (locks the MB) */
    cs = base->RAMn[mbOffset + 0];
    
    /* Read ID word */
    id = base->RAMn[mbOffset + 1];
    
    /* Extract message information */
    if (cs & CAN_WMBn_CS_IDE_MASK) {
        message->idType = CAN_ID_EXT;
        message->id = (id & CAN_ID_EXT_MASK) >> CAN_ID_EXT_SHIFT;
    } else {
        message->idType = CAN_ID_STD;
        message->id = (id & CAN_ID_STD_MASK) >> CAN_ID_STD_SHIFT;
    }
    
    message->frameType = (cs & CAN_WMBn_CS_RTR_MASK) ? CAN_FRAME_REMOTE : CAN_FRAME_DATA;
    message->dataLength = (cs & CAN_WMBn_CS_DLC_MASK) >> CAN_WMBn_CS_DLC_SHIFT;
    
    /* Read data words */
    data0 = base->RAMn[mbOffset + 2];
    data1 = base->RAMn[mbOffset + 3];
    
    /* Extract bytes from data words (big-endian) */
    message->data[0] = (uint8_t)((data0 >> 24) & 0xFFU);
    message->data[1] = (uint8_t)((data0 >> 16) & 0xFFU);
    message->data[2] = (uint8_t)((data0 >> 8) & 0xFFU);
    message->data[3] = (uint8_t)(data0 & 0xFFU);
    message->data[4] = (uint8_t)((data1 >> 24) & 0xFFU);
    message->data[5] = (uint8_t)((data1 >> 16) & 0xFFU);
    message->data[6] = (uint8_t)((data1 >> 8) & 0xFFU);
    message->data[7] = (uint8_t)(data1 & 0xFFU);
    
    /* Read TIMER to unlock message buffers */
    dummy = base->TIMER;
    (void)dummy;
    
    /* Clear interrupt flag */
    base->IFLAG1 = mbMask;
    
    /* Read free running timer to unlock MB */
    (void)base->TIMER;
    
    /* Clear interrupt flag */
    base->IFLAG1 = mbMask;
    
    return STATUS_SUCCESS;
}

/**
 * @brief Receive CAN message with timeout
 */
status_t CAN_ReceiveBlocking(uint8_t instance, uint8_t mbIndex, 
                             can_message_t *message, uint32_t timeoutMs)
{
    CAN_Type *base;
    uint32_t timeoutCount = 0;
    uint32_t mbMask;
    
    /* Validate parameters */
    if (instance >= CAN_INSTANCE_COUNT || message == NULL) {
        return STATUS_INVALID_PARAM;
    }
    
    if (!s_canInitialized[instance]) {
        return STATUS_NOT_INITIALIZED;
    }
    
    base = s_canBases[instance];
    mbMask = (1UL << mbIndex);
    
    /* Wait for message */
    while (timeoutCount < (timeoutMs * 1000U)) {
        if (base->IFLAG1 & mbMask) {
            return CAN_Receive(instance, mbIndex, message);
        }
        timeoutCount++;
    }
    
    return STATUS_TIMEOUT;
}

/**
 * @brief Configure RX filter
 */
status_t CAN_ConfigRxFilter(uint8_t instance, uint8_t mbIndex, 
                             const can_rx_filter_t *filter)
{
    CAN_Type *base;
    uint32_t mbOffset;
    uint32_t cs, id;
    
    /* Validate parameters */
    if (instance >= CAN_INSTANCE_COUNT || filter == NULL) {
        return STATUS_INVALID_PARAM;
    }
    
    if (!s_canInitialized[instance]) {
        return STATUS_NOT_INITIALIZED;
    }
    
    if (mbIndex < CAN_RX_MB_START || mbIndex >= CAN_MB_COUNT) {
        return STATUS_INVALID_PARAM;
    }
    
    base = s_canBases[instance];
    mbOffset = mbIndex * MSG_BUF_SIZE;
    
    /* Configure ID word */
    if (filter->idType == CAN_ID_EXT) {
        id = (filter->id << CAN_ID_EXT_SHIFT) & CAN_ID_EXT_MASK;
    } else {
        id = (filter->id << CAN_ID_STD_SHIFT) & CAN_ID_STD_MASK;
    }
    base->RAMn[mbOffset + 1] = id;
    
    /* Configure CS word as RX empty */
    cs = (CAN_CS_CODE_RX_EMPTY << CAN_CS_CODE_SHIFT);
    
    if (filter->idType == CAN_ID_EXT) {
        cs |= CAN_WMBn_CS_IDE_MASK;
    }
    
    base->RAMn[mbOffset + 0] = cs;
    
    /* Configure individual mask */
    base->RXIMR[mbIndex] = filter->mask;
    
    /* Enable interrupt for this MB */
    base->IMASK1 |= (1UL << mbIndex);

    return STATUS_SUCCESS;
}

/**
 * @brief Configure TX mailbox
 */
status_t CAN_ConfigTxMailbox(uint8_t instance, uint8_t mbIndex)
{
    CAN_Type *base;
    uint32_t mbOffset;
    
    /* Validate parameters */
    if (instance >= CAN_INSTANCE_COUNT) {
        return STATUS_INVALID_PARAM;
    }
    
    if (!s_canInitialized[instance]) {
        return STATUS_NOT_INITIALIZED;
    }
    
    if (mbIndex < CAN_TX_MB_START || 
        mbIndex >= (CAN_TX_MB_START + CAN_TX_MB_COUNT)) {
        return STATUS_INVALID_PARAM;
    }
    
    base = s_canBases[instance];
    mbOffset = mbIndex * MSG_BUF_SIZE;
    
    /* Set MB as inactive TX */
    base->RAMn[mbOffset + 0] = (CAN_CS_CODE_TX_INACTIVE << CAN_CS_CODE_SHIFT);
    
    /* Enable interrupt for this MB */
    base->IMASK1 |= (1UL << mbIndex);
    
    return STATUS_SUCCESS;
}

/* Legacy callback functions removed - use CAN_RegisterCallback() instead */

/**
 * @brief Get error state
 */
status_t CAN_GetErrorState(uint8_t instance, can_error_state_t *errorState)
{
    CAN_Type *base;
    uint32_t fltConf;
    
    if (instance >= CAN_INSTANCE_COUNT || errorState == NULL) {
        return STATUS_INVALID_PARAM;
    }
    
    base = s_canBases[instance];
    
    fltConf = (base->ESR1 & CAN_ESR1_FLTCONF_MASK) >> CAN_ESR1_FLTCONF_SHIFT;
    
    if (fltConf == 0U) {
        *errorState = CAN_ERROR_ACTIVE;
    } else if (fltConf == 1U) {
        *errorState = CAN_ERROR_PASSIVE;
    } else {
        *errorState = CAN_ERROR_BUS_OFF;
    }
    
    return STATUS_SUCCESS;
}

/**
 * @brief Get error counters
 */
status_t CAN_GetErrorCounters(uint8_t instance, 
                               uint8_t *txErrorCount, uint8_t *rxErrorCount)
{
    CAN_Type *base;
    
    if (instance >= CAN_INSTANCE_COUNT || 
        txErrorCount == NULL || rxErrorCount == NULL) {
        return STATUS_INVALID_PARAM;
    }
    
    base = s_canBases[instance];
    
    *txErrorCount = (base->ECR >> 8U) & 0xFFU;
    *rxErrorCount = base->ECR & 0xFFU;
    
    return STATUS_SUCCESS;
}

/**
 * @brief Abort transmission
 */
status_t CAN_AbortTransmission(uint8_t instance, uint8_t mbIndex)
{
    CAN_Type *base;
    uint32_t mbOffset;
    
    if (instance >= CAN_INSTANCE_COUNT || mbIndex >= CAN_MB_COUNT) {
        return STATUS_INVALID_PARAM;
    }
    
    base = s_canBases[instance];
    mbOffset = mbIndex * MSG_BUF_SIZE;
    
    /* Set abort code */
    base->RAMn[mbOffset + 0] = (CAN_CS_CODE_TX_ABORT << CAN_CS_CODE_SHIFT);
    
    return STATUS_SUCCESS;
}

/**
 * @brief Check if MB is busy
 */
status_t CAN_IsMbBusy(uint8_t instance, uint8_t mbIndex, bool *isBusy)
{
    CAN_Type *base;
    uint32_t mbOffset;
    uint32_t cs;
    uint32_t code;
    
    if (instance >= CAN_INSTANCE_COUNT || 
        mbIndex >= CAN_MB_COUNT || isBusy == NULL) {
        return STATUS_INVALID_PARAM;
    }
    
    base = s_canBases[instance];
    mbOffset = mbIndex * MSG_BUF_SIZE;
    
    cs = base->RAMn[mbOffset + 0];
    code = (cs & CAN_CS_CODE_MASK) >> CAN_CS_CODE_SHIFT;
    
    *isBusy = (code != CAN_CS_CODE_TX_INACTIVE &&
               code != CAN_CS_CODE_RX_INACTIVE);
    
    return STATUS_SUCCESS;
}

/**
 * @brief Calculate timing parameters
 */
status_t CAN_CalculateTiming(uint32_t canClockHz, uint32_t baudRate, 
                             can_timing_config_t *timing)
{
    uint32_t numTq;
    uint32_t preDiv;
    
    if (timing == NULL || baudRate == 0U || canClockHz == 0U) {
        return STATUS_INVALID_PARAM;
    }
    
    /* Target: 16 time quanta per bit for good timing */
    numTq = 16U;
    
    /* Calculate prescaler */
    preDiv = (canClockHz / (baudRate * numTq)) - 1U;
    
    if (preDiv > 255U) {
        /* Try with 8 time quanta */
        numTq = 8U;
        preDiv = (canClockHz / (baudRate * numTq)) - 1U;
        
        if (preDiv > 255U) {
            return STATUS_INVALID_PARAM;
        }
    }
    
    /* Configure timing with good margin */
    timing->preDiv = (uint8_t)preDiv;
    timing->preDiv = 0U;
    timing->propSeg = 6U;       /* Propagation segment */
    timing->phaseSeg1 = 3U;     /* Phase segment 1 */
    timing->phaseSeg2 = 3U;     /* Phase segment 2 */
    timing->rJumpWidth = 3U;    /* Resynchronization jump width */
    
    return STATUS_SUCCESS;
}



/*******************************************************************************
 * Private Functions
 ******************************************************************************/

/**
 * @brief Enter freeze mode
 */
static status_t CAN_EnterFreezeMode(CAN_Type *base)
{
    uint32_t timeout = CAN_FREEZE_TIMEOUT;
    
    /* Request freeze mode */
    base->MCR |= (CAN_MCR_FRZ_MASK | CAN_MCR_HALT_MASK);
    
    /* Wait for freeze mode acknowledge */
    while ((timeout > 0U) && ((base->MCR & CAN_MCR_FRZACK_MASK) == 0U)) {
        timeout--;
    }
    
    if (timeout == 0U) {
        return STATUS_TIMEOUT;
    }
    
    return STATUS_SUCCESS;
}

/**
 * @brief Exit freeze mode
 */
static status_t CAN_ExitFreezeMode(CAN_Type *base)
{
    uint32_t timeout = CAN_FREEZE_TIMEOUT;
    
    /* Clear freeze and halt bits */
    base->MCR &= ~(CAN_MCR_FRZ_MASK | CAN_MCR_HALT_MASK);
    
    /* Wait for module ready */
    while ((timeout > 0U) && ((base->MCR & CAN_MCR_FRZACK_MASK) != 0U)) {
        timeout--;
    }
    
    if (timeout == 0U) {
        return STATUS_TIMEOUT;
    }
    
    /* Wait for module not ready flag to clear */
    timeout = CAN_FREEZE_TIMEOUT;
    while ((timeout > 0U) && ((base->MCR & CAN_MCR_NOTRDY_MASK) != 0U)) {
        timeout--;
    }
    
    if (timeout == 0U) {
        return STATUS_TIMEOUT;
    }
    
    return STATUS_SUCCESS;
}

/**
 * @brief Soft reset CAN module
 */
static status_t CAN_SoftReset(CAN_Type *base)
{
    uint32_t timeout = CAN_FREEZE_TIMEOUT;
    
    /* Request soft reset */
    base->MCR |= CAN_MCR_SOFTRST_MASK;
    
    /* Wait for reset to complete */
    while ((timeout > 0U) && ((base->MCR & CAN_MCR_SOFTRST_MASK) != 0U)) {
        timeout--;
    }
    
    if (timeout == 0U) {
        return STATUS_TIMEOUT;
    }
    
    return STATUS_SUCCESS;
}

/**
 * @brief Enable CAN peripheral clock
 */
static void CAN_EnableClock(uint8_t instance, can_clk_src_t clockSource)
{
//	DEV_ASSERT(instance > 2U);
    CAN_Type *base;
    
    base = s_canBases[instance];
    // /* Get PCC register address */
    // if (instance == 0U) {
    //     PCC->PCCn[PCC_FLEXCAN0_INDEX] |= PCC_PCCn_CGC_MASK;
    // } else if (instance == 1U) {
    // 	PCC->PCCn[PCC_FLEXCAN1_INDEX] |= PCC_PCCn_CGC_MASK;
    // } else {
    // 	PCC->PCCn[PCC_FLEXCAN2_INDEX] |= PCC_PCCn_CGC_MASK;
    // }

    /* MDIS=1: Disable module before selecting clock */
    base->MCR 	|= CAN_MCR_MDIS_MASK; 

    /* Select clock source in CTRL1*/ 
    if(clockSource == CAN_CLK_SRC_SOSCDIV2){
    	base->CTRL1 &=~ CAN_CTRL1_CLKSRC_MASK; /* CLKSRC=0: Clock Source = oscillator (8 MHz) */
    }else{
    	base->CTRL1 |= CAN_CTRL1_CLKSRC_MASK; /* CLKSRC=1: Clock Source = oscillator (8 MHz) */
    }

    /* MDIS=0; Enable module config. (Sets FRZ, HALT)*/
    base->MCR 	&= ~CAN_MCR_MDIS_MASK; 


}

/**
 * @brief Initialize all Message Buffers to inactive
 */
static void CAN_InitMessageBuffers(CAN_Type *base)
{
    uint32_t i;
    
    /* Clear all 32 msg bufs x 4 words/msg buf = 128 words */
    for (i = 0; i < 128; i++) {
        base->RAMn[i] = 0;
    }
    
    /* In FRZ mode, init CAN 16 msg buf filters */
    for (i = 0; i < 16; i++) {
        /* Check all ID bits for incoming messages */
        base->RXIMR[i] = 0xFFFFFFFFUL;
    }
    
    /* Clear all interrupt flags 
        This reg is w1c*/
    base->IFLAG1 = 0xFFFFFFFFUL;
}


/**
 * @brief Get clock frequency from clock source
 */
static uint32_t CAN_GetClockFrequency(can_clk_src_t clockSource)
{
    uint32_t freq;
    
    switch (clockSource) {
        case CAN_CLK_SRC_SOSCDIV2:
            freq = 4000000U;    /* 8 MHz / 2 = 4 MHz typical */
            break;
        case CAN_CLK_SRC_BUSCLOCK:
            freq = 40000000U;   /* 80 MHz / 2 = 40 MHz typical */
            break;
        default:
            freq = CAN_DEFAULT_CLK_FREQ;
            break;
    }
    
    return freq;
}

/*******************************************************************************
 * New Interrupt Pattern API Implementation
 ******************************************************************************/

/**
 * @brief Get CAN instance index
 */
static uint8_t CAN_GetInstanceIndex(CAN_Type *base)
{
    if (base == CAN0) {
        return 0U;
    } else if (base == CAN1) {
        return 1U;
    } else if (base == CAN2) {
        return 2U;
    }
    return 0xFFU;
}

/**
 * @brief Register unified callback
 */
status_t CAN_RegisterCallback(CAN_Type *instance, can_callback_t callback)
{
    uint8_t index = CAN_GetInstanceIndex(instance);
    
    if (index == 0xFFU) {
        return STATUS_INVALID_PARAM;
    }
    
    s_canCallbacks[index] = callback;
    return STATUS_SUCCESS;
}

/**
 * @brief Unregister callback
 */
status_t CAN_UnregisterCallback(CAN_Type *instance)
{
    uint8_t index = CAN_GetInstanceIndex(instance);
    
    if (index == 0xFFU) {
        return STATUS_INVALID_PARAM;
    }
    
    s_canCallbacks[index] = NULL;
    return STATUS_SUCCESS;
}

/**
 * @brief CAN IRQ Handler
 * @details Handles all CAN interrupts and invokes registered callback
 */
void CAN_IRQHandler(CAN_Type *instance)
{
    uint8_t instIdx = CAN_GetInstanceIndex(instance);
    
    if (instIdx == 0xFFU || s_canCallbacks[instIdx] == NULL) {
        return;
    }
    
    can_event_t event = CAN_EVENT_NONE;
    can_event_data_t eventData = {0};
    can_message_t rxMessage = {0};
    
    /* Check MB interrupts (TX/RX) */
    uint32_t iflag1 = instance->IFLAG1;
    
    if (iflag1 != 0) {
        /* Find first set bit (lowest MB with interrupt) */
        for (uint8_t mbIdx = 0; mbIdx < 32U; mbIdx++) {
            if ((iflag1 & (1UL << mbIdx)) != 0) {
                /* Read CS to determine if TX or RX */
                uint32_t cs = CAN_ReadMbCs(instance, mbIdx);
                uint8_t code = (cs & CAN_CS_CODE_MASK) >> CAN_CS_CODE_SHIFT;
                
                if (code == CAN_CS_CODE_TX_INACTIVE) {
                    /* TX Complete */
                    event = CAN_EVENT_TX_COMPLETE;
                    eventData.mbIndex = mbIdx;
                    
                    /* Clear flag */
                    instance->IFLAG1 = (1UL << mbIdx);
                    
                } else if (code == CAN_CS_CODE_RX_FULL) {
                    /* RX Complete - read message */
                    event = CAN_EVENT_RX_COMPLETE;
                    eventData.mbIndex = mbIdx;
                    
                    /* Extract message data */
                    uint8_t dlc = (cs & CAN_WMBn_CS_DLC_MASK) >> CAN_WMBn_CS_DLC_SHIFT;
                    rxMessage.dataLength = (dlc > 8U) ? 8U : dlc;
                    
                    /* Extract ID */
                    uint32_t idReg = CAN_ReadMbId(instance, mbIdx);
                    if ((cs & CAN_WMBn_CS_IDE_MASK) != 0) {
                        rxMessage.idType = CAN_ID_EXT;
                        rxMessage.id = (idReg & CAN_ID_EXT_MASK) >> CAN_ID_EXT_SHIFT;
                    } else {
                        rxMessage.idType = CAN_ID_STD;
                        rxMessage.id = (idReg & CAN_ID_STD_MASK) >> CAN_ID_STD_SHIFT;
                    }
                    
                    /* Extract frame type */
                    rxMessage.frameType = ((cs & CAN_WMBn_CS_RTR_MASK) != 0) ? 
                                         CAN_FRAME_REMOTE : CAN_FRAME_DATA;
                    
                    /* Copy data */
                    CAN_CopyDataFromMb(instance, mbIdx, rxMessage.data, rxMessage.dataLength);
                    
                    eventData.message = &rxMessage;
                    
                    /* Clear flag (after reading data) */
                    instance->IFLAG1 = (1UL << mbIdx);
                    
                    /* Reactivate MB for next reception */
                    CAN_WriteMbCs(instance, mbIdx, 
                                 (CAN_CS_CODE_RX_EMPTY << CAN_CS_CODE_SHIFT) | 
                                 (cs & (CAN_WMBn_CS_IDE_MASK | CAN_WMBn_CS_RTR_MASK)));
                }
                
                /* Call unified callback */
                if (event != CAN_EVENT_NONE) {
                    s_canCallbacks[instIdx](instance, event, &eventData);
                }
                
                /* Process only first interrupt */
                break;
            }
        }
    }
    
    /* Error handling disabled temporarily */
    /* uint32_t esr1 = instance->ESR1;
    
    if ((esr1 & CAN_ESR1_BOFFINT_MASK) != 0) {
        event = CAN_EVENT_BUS_OFF;
        eventData.errorFlags = esr1;
        instance->ESR1 = CAN_ESR1_BOFFINT_MASK;
        s_canCallbacks[instIdx](instance, event, &eventData);
    }
    
    if ((esr1 & CAN_ESR1_ERRINT_MASK) != 0) {
        event = CAN_EVENT_ERROR;
        eventData.errorFlags = esr1;
        instance->ESR1 = CAN_ESR1_ERRINT_MASK;
        s_canCallbacks[instIdx](instance, event, &eventData);
    } */
}
