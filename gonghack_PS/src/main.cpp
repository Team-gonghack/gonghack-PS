#include "xparameters.h"
#include "xil_io.h"
#include "sleep.h"
#include <stdio.h>
#include <stdint.h>
#include "xaxidma.h"
#include "xil_cache.h"
#include "xaxidma_hw.h"

#define I2C_READER_0_BASEADDR  0x43C60000 
#define I2C_READER_1_BASEADDR  0x43C70000 
#define I2C_READER_2_BASEADDR  0x43C80000 
#define I2C_READER_3_BASEADDR  0x43C90000 
#define I2C_READER_4_BASEADDR  0x43CA0000 

#define DMA_CTRL_BASEADDR      XPAR_AXI_DMA_0_BASEADDR
#define UARTLITE_BASEADDR      XPAR_AXI_UARTLITE_0_BASEADDR

#define REG_START_TRIGGER 0x00
#define NUM_READERS       5    
#define DATA_WIDTH_BYTES  8    
#define DATA_SIZE_BYTES   (NUM_READERS * DATA_WIDTH_BYTES)
#define RX_ALIGNMENT      64   

#define UARTLITE_TX_FIFO_OFFSET  0x4
#define UARTLITE_STAT_REG_OFFSET 0x8
#define UARTLITE_STAT_TX_FULL    0x08 

uint64_t RxBuffer[NUM_READERS] __attribute__ ((aligned(RX_ALIGNMENT)));

XAxiDma AxiDma;

int main() {
    xil_printf("==== I2C Reader DMA Control Start ====\r\n");

    uint32_t readers[NUM_READERS] = {
        I2C_READER_0_BASEADDR, I2C_READER_1_BASEADDR, I2C_READER_2_BASEADDR, 
        I2C_READER_3_BASEADDR, I2C_READER_4_BASEADDR
    };
    
    uint64_t *RxBufferPtr = (uint64_t *)RxBuffer;
    int Status;
    int Index;
    
    XAxiDma_Config *CfgPtr = XAxiDma_LookupConfig(DMA_CTRL_BASEADDR);
    if (!CfgPtr) {
        xil_printf("DMA Cfg Lookup FAILED\r\n");
        return XST_FAILURE;
    }
    Status = XAxiDma_CfgInitialize(&AxiDma, CfgPtr);
    if (Status != XST_SUCCESS) {
        xil_printf("DMA Init FAILED\r\n");
        return XST_FAILURE;
    }
    
    if (XAxiDma_ResetIsDone(&AxiDma) == 0) {
         xil_printf("DMA ResetIsDone FAILED (Not Ready).\r\n");
         XAxiDma_Reset(&AxiDma);
         sleep(1);
         if(XAxiDma_ResetIsDone(&AxiDma) == 0) {
            xil_printf("DMA Reset FAILED again.\r\n");
            return XST_FAILURE;
         }
    }
    xil_printf("DMA S2MM Channel is ready. Entering main loop...\r\n");

    while(1) {
        
        Xil_DCacheFlushRange((UINTPTR)RxBufferPtr, DATA_SIZE_BYTES);

        Status = XAxiDma_SimpleTransfer(&AxiDma, (UINTPTR)RxBufferPtr, DATA_SIZE_BYTES, XAXIDMA_DEVICE_TO_DMA);
        if (Status != XST_SUCCESS) {
            xil_printf("DMA Transfer Setup FAILED\r\n");
            continue; 
        }
        
        for (int i = 0; i < NUM_READERS; i++) {
            Xil_Out32(readers[i] + REG_START_TRIGGER, 0x1);
            Xil_Out32(readers[i] + REG_START_TRIGGER, 0x0);
        }
        
        while (XAxiDma_Busy(&AxiDma, XAXIDMA_DEVICE_TO_DMA)) {
            
        }
        
        uint32_t dma_status = XAxiDma_ReadReg(AxiDma.RegBase + XAXIDMA_RX_OFFSET, XAXIDMA_SR_OFFSET);
        if ((dma_status & XAXIDMA_ERR_ALL_MASK) != 0) {
            xil_printf("DMA S2MM Error Detected: 0x%08X\r\n", dma_status);
            XAxiDma_Reset(&AxiDma);
            sleep(1);
            continue;
        }

        Xil_DCacheInvalidateRange((UINTPTR)RxBufferPtr, DATA_SIZE_BYTES);
        xil_printf("DMA Transfer Complete. Reading Data:\r\n");

        for (Index = 0; Index < NUM_READERS; Index++) {
            uint64_t data64 = RxBufferPtr[Index];  
            
            int16_t accel_x = (int16_t)((data64 >> 32) & 0xFFFF); 
            int16_t accel_y = (int16_t)((data64 >> 16) & 0xFFFF); 
            int16_t accel_z = (int16_t)(data64 & 0xFFFF);         
            
            xil_printf("MPU Data #%d: X=%d, Y=%d, Z=%d (RAW 0x%llx)\r\n", Index, accel_x, accel_y, accel_z, data64);
        }

        uint8_t *BytePtr = (uint8_t *)RxBufferPtr;
        
        for (Index = 0; Index < DATA_SIZE_BYTES; Index++) {
            while (Xil_In32(UARTLITE_BASEADDR + UARTLITE_STAT_REG_OFFSET) & UARTLITE_STAT_TX_FULL) {
                
            }
            Xil_Out32(UARTLITE_BASEADDR + UARTLITE_TX_FIFO_OFFSET, BytePtr[Index]);
        }
        xil_printf("PL UART (axi_uartlite_0) Send Complete.\r\n");

        xil_printf("---- Cycle Complete ----\r\n");
        usleep(100000); 
    }

    return 0; 
}
