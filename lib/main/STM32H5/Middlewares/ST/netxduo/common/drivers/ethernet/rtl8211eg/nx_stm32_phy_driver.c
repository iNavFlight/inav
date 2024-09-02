
/**************************************************************************/
/*                                                                        */
/*       Copyright (c) Microsoft Corporation. All rights reserved.        */
/*                                                                        */
/*       This software is licensed under the Microsoft Software License   */
/*       Terms for Microsoft Azure RTOS. Full text of the license can be  */
/*       found in the LICENSE file at https://aka.ms/AzureRTOS_EULA       */
/*       and in the root directory of this software.                      */
/*                                                                        */
/**************************************************************************/


#include "nx_stm32_phy_driver.h"
#include "nx_stm32_eth_config.h"


/* RTL8211EG IO functions */
static int32_t rtl8211eg_io_init(void);
static int32_t rtl8211eg_io_deinit (void);

static int32_t rtl8211eg_io_write_reg(uint32_t DevAddr, uint32_t RegAddr, uint32_t RegVal);
static int32_t rtl8211eg_io_read_reg(uint32_t DevAddr, uint32_t RegAddr, uint32_t *pRegVal);

static int32_t rtl8211eg_io_get_tick(void);

/* RTL8211EG IO context object */
static rtl8211eg_IOCtx_t  RTL8211EG_IOCtx = { rtl8211eg_io_init,
                                          rtl8211eg_io_deinit,
                                          rtl8211eg_io_write_reg,
                                          rtl8211eg_io_read_reg,
                                          rtl8211eg_io_get_tick
                                        };
/* RTL8211EG main object */
static rtl8211eg_Object_t RTL8211EG;

/**
  * @brief  Initialize the PHY interface
  * @param  none
  * @retval ETH_PHY_STATUS_OK on success, ETH_PHY_STATUS_ERROR otherwise
  */

int32_t nx_eth_phy_init(void)
{
    int32_t ret = ETH_PHY_STATUS_ERROR;
    /* Set PHY IO functions */

    RTL8211EG_RegisterBusIO(&RTL8211EG, &RTL8211EG_IOCtx);
    /* Initialize the RTL8211EG ETH PHY */

    if (RTL8211EG_Init(&RTL8211EG) == RTL8211EG_STATUS_OK)
    {
        ret = ETH_PHY_STATUS_OK;
    }

    return ret;
}

/**
  * @brief  set the Phy link state.
  * @param  LinkState
  * @retval the link status.
  */

int32_t nx_eth_phy_set_link_state(int32_t LinkState)
{
   return (RTL8211EG_SetLinkState(&RTL8211EG, LinkState));
}

/**
  * @brief  get the Phy link state.
  * @param  none
  * @retval the link status.
  */

int32_t nx_eth_phy_get_link_state(void)
{
    int32_t  linkstate = RTL8211EG_GetLinkState(&RTL8211EG);

    return linkstate;
}

/**
  * @brief  get the driver object handle
  * @param  none
  * @retval pointer to the RTL8211EG main object
  */

nx_eth_phy_handle_t nx_eth_phy_get_handle(void)
{
    return (nx_eth_phy_handle_t)&RTL8211EG;
}

/**
  * @brief  Initialize the PHY MDIO interface
  * @param  None
  * @retval 0 if OK, -1 if ERROR
  */

int32_t rtl8211eg_io_init(void)
{
  /* We assume that MDIO GPIO configuration is already done
     in the ETH_MspInit() else it should be done here
  */

  /* Configure the MDIO Clock */
  HAL_ETH_SetMDIOClockRange(&eth_handle);

  return ETH_PHY_STATUS_OK;
}

/**
  * @brief  De-Initialize the MDIO interface
  * @param  None
  * @retval 0 if OK, -1 if ERROR
  */
int32_t rtl8211eg_io_deinit (void)
{
    return ETH_PHY_STATUS_OK;
}

/**
  * @brief  Read a PHY register through the MDIO interface.
  * @param  DevAddr: PHY port address
  * @param  RegAddr: PHY register address
  * @param  pRegVal: pointer to hold the register value
  * @retval 0 if OK -1 if Error
  */
int32_t rtl8211eg_io_read_reg(uint32_t DevAddr, uint32_t RegAddr, uint32_t *pRegVal)
{
  if(HAL_ETH_ReadPHYRegister(&eth_handle, DevAddr, RegAddr, pRegVal) != HAL_OK)
  {
    return ETH_PHY_STATUS_ERROR;
  }

  return ETH_PHY_STATUS_OK;
}

int32_t rtl8211eg_io_write_reg(uint32_t DevAddr, uint32_t RegAddr, uint32_t RegVal)
{
  if(HAL_ETH_WritePHYRegister(&eth_handle, DevAddr, RegAddr, RegVal) != HAL_OK)
  {
    return ETH_PHY_STATUS_ERROR;
  }

  return ETH_PHY_STATUS_OK;
}

/**
  * @brief  Get the time in millisecons used for internal PHY driver process.
  * @retval Time value
  */
int32_t rtl8211eg_io_get_tick(void)
{
  return HAL_GetTick();
}
