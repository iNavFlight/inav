
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


/* LAN8742 IO functions */
static int32_t lan8742_io_init(void);
static int32_t lan8742_io_deinit (void);

static int32_t lan8742_io_write_reg(uint32_t DevAddr, uint32_t RegAddr, uint32_t RegVal);
static int32_t lan8742_io_read_reg(uint32_t DevAddr, uint32_t RegAddr, uint32_t *pRegVal);

static int32_t lan8742_io_get_tick(void);

/* LAN8742 IO context object */
static lan8742_IOCtx_t  LAN8742_IOCtx = { lan8742_io_init,
                                          lan8742_io_deinit,
                                          lan8742_io_write_reg,
                                          lan8742_io_read_reg,
                                          lan8742_io_get_tick
                                        };
/* LAN8742 main object */
static lan8742_Object_t LAN8742;

/**
  * @brief  Initialize the PHY interface
  * @param  none
  * @retval ETH_PHY_STATUS_OK on success, ETH_PHY_STATUS_ERROR otherwise
  */

int32_t nx_eth_phy_init(void)
{
    int32_t ret = ETH_PHY_STATUS_ERROR;
    /* Set PHY IO functions */

    LAN8742_RegisterBusIO(&LAN8742, &LAN8742_IOCtx);
    /* Initialize the LAN8742 ETH PHY */

    if (LAN8742_Init(&LAN8742) == LAN8742_STATUS_OK)
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
    return (LAN8742_SetLinkState(&LAN8742, LinkState));
}

/**
  * @brief  get the Phy link state.
  * @param  none
  * @retval the link status.
  */

int32_t nx_eth_phy_get_link_state(void)
{
    int32_t  linkstate = LAN8742_GetLinkState(&LAN8742);

    return linkstate;
}

/**
  * @brief  get the driver object handle
  * @param  none
  * @retval pointer to the LAN8742 main object
  */

nx_eth_phy_handle_t nx_eth_phy_get_handle(void)
{
    return (nx_eth_phy_handle_t)&LAN8742;
}

/**
  * @brief  Initialize the PHY MDIO interface
  * @param  None
  * @retval 0 if OK, -1 if ERROR
  */

int32_t lan8742_io_init(void)
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
int32_t lan8742_io_deinit (void)
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
int32_t lan8742_io_read_reg(uint32_t DevAddr, uint32_t RegAddr, uint32_t *pRegVal)
{
  if(HAL_ETH_ReadPHYRegister(&eth_handle, DevAddr, RegAddr, pRegVal) != HAL_OK)
  {
    return ETH_PHY_STATUS_ERROR;
  }

  return ETH_PHY_STATUS_OK;
}

int32_t lan8742_io_write_reg(uint32_t DevAddr, uint32_t RegAddr, uint32_t RegVal)
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
int32_t lan8742_io_get_tick(void)
{
  return HAL_GetTick();
}
