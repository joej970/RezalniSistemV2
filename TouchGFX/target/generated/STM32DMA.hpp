/**
  ******************************************************************************
  * File Name          : STM32DMA.hpp
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */

#ifndef STM32F7DMA_HPP
#define STM32F7DMA_HPP

#include <touchgfx/Bitmap.hpp>
#include <touchgfx/hal/DMA.hpp>

/**
 * @class STM32F7DMA
 *
 * @brief This class specializes DMA_Interface for the STM32F7 processors.
 *
 * @sa touchgfx::DMA_Interface
 */
class STM32F7DMA : public touchgfx::DMA_Interface
{
    /**
     * @typedef touchgfx::DMA_Interface Base
     *
     * @brief Defines an alias representing the base.
     *
     Defines an alias representing the base.
     */
    typedef touchgfx::DMA_Interface Base;

public:
    /**
     * @fn STM32F7DMA::STM32F7DMA();
     *
     * @brief Default constructor.
     *
     *        Default constructor.
     */
    STM32F7DMA();

    /**
     * @fn STM32F7DMA::~STM32F7DMA();
     *
     * @brief Destructor.
     *
     *        Destructor.
     */
    virtual ~STM32F7DMA();

    /**
     * @fn DMAType touchgfx::STM32F7DMA::getDMAType()
     *
     * @brief Function for obtaining the DMA type of the concrete DMA_Interface implementation.
     *
     *        Function for obtaining the DMA type of the concrete DMA_Interface implementation.
     *        As default, will return DMA_TYPE_CHROMART type value.
     *
     * @return a DMAType value of the concrete DMA_Interface implementation.
     */
    virtual touchgfx::DMAType getDMAType(void)
    {
        return touchgfx::DMA_TYPE_CHROMART;
    }

    /**
     * @fn touchgfx::BlitOperations STM32F7DMA::getBlitCaps();
     *
     * @brief Gets the blit capabilities.
     *
     *        Gets the blit capabilities.
     *
     *        This DMA supports a range of blit caps: BLIT_OP_COPY, BLIT_OP_COPY_ARGB8888,
     *        BLIT_OP_COPY_ARGB8888_WITH_ALPHA, BLIT_OP_COPY_A4, BLIT_OP_COPY_A8.
     *
     *
     * @return Currently supported blitcaps.
     */
    virtual touchgfx::BlitOperations getBlitCaps();

    /**
     * @fn void STM32F7DMA::initialize();
     *
     * @brief Perform hardware specific initialization.
     *
     *        Perform hardware specific initialization.
     */
    virtual void initialize();

    /**
     * @fn void STM32F7DMA::signalDMAInterrupt()
     *
     * @brief Raises a DMA interrupt signal.
     *
     *        Raises a DMA interrupt signal.
     */
    virtual void signalDMAInterrupt()
    {
        executeCompleted();
    }

protected:
    /**
     * @fn virtual void STM32F7DMA::setupDataCopy(const touchgfx::BlitOp& blitOp);
     *
     * @brief Configures the DMA for copying data to the frame buffer.
     *
     *        Configures the DMA for copying data to the frame buffer.
     *
     * @param blitOp Details on the copy to perform.
     */
    virtual void setupDataCopy(const touchgfx::BlitOp& blitOp);

    /**
     * @fn virtual void STM32F7DMA::setupDataFill(const touchgfx::BlitOp& blitOp);
     *
     * @brief Configures the DMA for "filling" the frame-buffer with a single color.
     *
     *        Configures the DMA for "filling" the frame-buffer with a single color.
     *
     * @param blitOp Details on the "fill" to perform.
     */
    virtual void setupDataFill(const touchgfx::BlitOp& blitOp);

private:
    touchgfx::LockFreeDMA_Queue dma_queue;
    touchgfx::BlitOp queue_storage[96];

    /**
     * @fn void STM32F7DMA::getChromARTInputFormat()
     *
     * @brief Convert Bitmap format to ChromART Input format.
     *
     * @param format Bitmap format.
     *
     * @return ChromART Input format.
     */

    inline uint32_t getChromARTInputFormat(touchgfx::Bitmap::BitmapFormat format);

    /**
     * @fn void STM32F7DMA::getChromARTOutputFormat()
     *
     * @brief Convert Bitmap format to ChromART Output format.
     *
     * @param format Bitmap format.
     *
     * @return ChromART Output format.
     */
    inline uint32_t getChromARTOutputFormat(touchgfx::Bitmap::BitmapFormat format);
};

#endif // STM32F7DMA_HPP
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
