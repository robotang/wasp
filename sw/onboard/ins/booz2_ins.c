/*
 * Copyright (C) 2008 Antoine Drouin
 * Copyright (C) 2009 John Stowers
 *
 * This file is part of wasp, some code taken from paparazzi (GPL)
 *
 * wasp is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * wasp is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with paparazzi; see the file COPYING.  If not, write to
 * the Free Software Foundation, 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 */
#include "ins.h"
#include "imu.h"
#include "altimeter.h"
#include "gps.h"
#include "ahrs.h"
#include "math/pprz_algebra_int.h"
#include "math/pprz_geodetic_int.h"

/* FIXME: Conditional includes are soo 1984, that is
   why generated/settings must be included first, it defines 
   CONTROL_USE_VFF */
#include "generated/settings.h"
#if CONTROL_USE_VFF
#include "ins/booz2_vf_float.h"
#endif
#include "ins/booz2_hf_float.h"

INS_t   ins;

void ins_init() {
#if CONTROL_USE_VFF
  ins.ltp_initialised  = FALSE;
  ins.baro_initialised = FALSE;
  ins.vff_realign = FALSE;
  b2_vff_init(0., 0., 0.);
#endif
  b2ins_init();
  INT32_VECT3_ZERO(ins.enu_pos);
  INT32_VECT3_ZERO(ins.enu_speed);
  INT32_VECT3_ZERO(ins.enu_accel);
}

void ins_propagate() {

#if CONTROL_USE_VFF
  if (altimeter_system_status == STATUS_INITIALIZED && ins.baro_initialised) {
    float accel_float = ACCEL_FLOAT_OF_BFP(booz_imu.accel.z);
    b2_vff_propagate(accel_float);
    ins.ltp_accel.z = ACCEL_BFP_OF_REAL(b2_vff_zdotdot);
    ins.ltp_speed.z = SPEED_BFP_OF_REAL(b2_vff_zdot);
    ins.ltp_pos.z = POS_BFP_OF_REAL(b2_vff_z);
    ins.enu_pos.z = -ins.ltp_pos.z;
    ins.enu_speed.z = -ins.ltp_speed.z;
    ins.enu_accel.z = -ins.ltp_accel.z;
  }
#endif
#ifdef USE_HFF
  if (booz_ahrs.status == BOOZ_AHRS_RUNNING &&
      gps_state.fix == BOOZ2_GPS_FIX_3D && ins.ltp_initialised )
    b2ins_propagate();
#endif
}

void ins_update_baro() {

#if CONTROL_USE_VFF
  if (altimeter_system_status == STATUS_INITIALIZED) {
    uint32_t alt = altimeter_get_altitude();
    if (!ins.baro_initialised) {
      ins.qfe = alt;
      ins.baro_initialised = TRUE;
    }
    ins.baro_alt = alt - ins.qfe;
    float alt_float = POS_FLOAT_OF_BFP(ins.baro_alt);
    if (ins.vff_realign) {
      ins.vff_realign = FALSE;
      ins.qfe = alt;
      b2_vff_realign(0.);
    }
    b2_vff_update(alt_float);
  }
#endif
}


void ins_update_gps(void) {

  if (gps_state.fix == GPS_FIX_3D) {
    if (!ins.ltp_initialised) {
      ltp_def_from_ecef_i(&ins.ltp_def, &gps_state.ecef_pos);
      ins.ltp_initialised = TRUE;
    }
    ned_of_ecef_point_i(&ins.gps_pos_cm_ned, &ins.ltp_def, &gps_state.ecef_pos);
    ned_of_ecef_vect_i(&ins.gps_speed_cm_s_ned, &ins.ltp_def, &gps_state.ecef_speed);
#ifdef USE_HFF
    b2ins_update_gps();
    VECT2_SDIV(ins.ltp_pos, (1<<(B2INS_POS_LTP_FRAC-INT32_POS_FRAC)), b2ins_pos_ltp);
    VECT2_SDIV(ins.ltp_speed, (1<<(B2INS_SPEED_LTP_FRAC-INT32_SPEED_FRAC)), b2ins_speed_ltp);
#else
    INT32_VECT3_SCALE_2(b2ins_meas_gps_pos_ned, ins.gps_pos_cm_ned, 
		INT32_POS_OF_CM_NUM, INT32_POS_OF_CM_DEN); 
    INT32_VECT3_SCALE_2(b2ins_meas_gps_speed_ned, ins.gps_speed_cm_s_ned,
		INT32_SPEED_OF_CM_S_NUM, INT32_SPEED_OF_CM_S_DEN); 
    VECT3_COPY(ins.ltp_pos,   b2ins_meas_gps_pos_ned);
    VECT3_COPY(ins.ltp_speed, b2ins_meas_gps_speed_ned);
#endif
    INT32_VECT3_ENU_OF_NED(ins.enu_pos, ins.ltp_pos);
    INT32_VECT3_ENU_OF_NED(ins.enu_speed, ins.ltp_speed);
    INT32_VECT3_ENU_OF_NED(ins.enu_accel, ins.ltp_accel);
  }

}


