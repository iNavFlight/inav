/**
  ******************************************************************************
  * @file    net_state.h
  * @author  MCD Application Team
  * @brief   Header for the network state management functions
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */

#ifndef NET_STATE_H
#define NET_STATE_H
int32_t net_state_manage_event(net_if_handle_t *pnetif, net_state_event_t state_to);
void net_state_transition_done(net_if_handle_t *pnetif, net_state_t state_to);

#endif /* NET_STATE_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
