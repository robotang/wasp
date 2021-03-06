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
#define NAV_C

#include "ins.h"
#include "booz2_navigation.h"
#include "math/pprz_algebra_int.h"

#include "config/flight_plan.h"


const uint8_t nb_waypoint = NB_WAYPOINT;
struct EnuCoor_i waypoints[NB_WAYPOINT] = WAYPOINTS_INT32;

struct EnuCoor_i booz2_navigation_target;
struct EnuCoor_i booz2_navigation_carrot;

struct EnuCoor_i nav_last_point;

uint16_t stage_time, block_time;

uint8_t nav_stage, nav_block;
uint8_t last_block, last_stage;
uint8_t last_wp __attribute__ ((unused));

int32_t ground_alt, nav_altitude;

uint8_t horizontal_mode;
uint8_t nav_segment_start, nav_segment_end;

#define ARRIVED_AT_WAYPOINT (15 << 8)
#define CARROT_DIST (10 << 8)

void booz2_nav_init(void) {
  nav_block = 0;
  nav_stage = 0;
  ground_alt = (int32_t)(GROUND_ALT * 100); // cm
  nav_altitude = ground_alt + SECURITY_HEIGHT;
  INT32_VECT3_COPY( booz2_navigation_target, waypoints[WP_HOME]);
  INT32_VECT3_COPY( booz2_navigation_carrot, waypoints[WP_HOME]);

  horizontal_mode = HORIZONTAL_MODE_WAYPOINT;
}

void booz2_nav_run(void) {

  /* compute a vector to the waypoint */
  struct Int32Vect2 path_to_waypoint;
  VECT2_DIFF(path_to_waypoint, booz2_navigation_target, ins.enu_pos);

  /* saturate it */
  VECT2_STRIM(path_to_waypoint, -(1<<15), (1<<15));

  int32_t dist_to_waypoint;
  INT32_VECT2_NORM(dist_to_waypoint, path_to_waypoint);
  
  if (dist_to_waypoint < ARRIVED_AT_WAYPOINT) {
    VECT2_COPY( booz2_navigation_carrot, booz2_navigation_target);
  }
  else {
    struct Int32Vect2 path_to_carrot;
    VECT2_SMUL(path_to_carrot, CARROT_DIST, path_to_waypoint);
    VECT2_SDIV(path_to_carrot, dist_to_waypoint, path_to_carrot);
    VECT2_SUM(booz2_navigation_carrot, path_to_carrot, ins.enu_pos);
  }
}

//#include "stdio.h"
void nav_route(uint8_t wp_start, uint8_t wp_end) {
  struct Int32Vect2 wp_diff,pos_diff;
  VECT2_DIFF(wp_diff, waypoints[wp_end],waypoints[wp_start]);
  VECT2_DIFF(pos_diff, ins.enu_pos,waypoints[wp_start]);
  // go back to metric precision or values are too large
  INT32_VECT2_RSHIFT(wp_diff,wp_diff,INT32_POS_FRAC);
  INT32_VECT2_RSHIFT(pos_diff,pos_diff,INT32_POS_FRAC);
  int32_t leg_length;
  int32_t leg_length2 = Max((wp_diff.x * wp_diff.x + wp_diff.y * wp_diff.y),1);
  INT32_SQRT(leg_length,leg_length2);
  int32_t nav_leg_progress = (pos_diff.x * wp_diff.x + pos_diff.y * wp_diff.y) / leg_length;
  nav_leg_progress += Max((CARROT_DIST >> INT32_POS_FRAC), 0);
  struct Int32Vect2 progress_pos;
  VECT2_SMUL(progress_pos, nav_leg_progress, wp_diff);
  VECT2_SDIV(progress_pos, leg_length, progress_pos);
  INT32_VECT2_LSHIFT(progress_pos,progress_pos,INT32_POS_FRAC);
  VECT2_SUM(booz2_navigation_target,waypoints[wp_start],progress_pos);
  //printf("target %d %d | p %d %d | s %d %d | l %d %d %d\n",
  //    booz2_navigation_target.x,
  //    booz2_navigation_target.y,
  //    progress_pos.x,
  //    progress_pos.y,
  //    waypoints[wp_start].x,
  //    waypoints[wp_start].y,
  //    leg_length, leg_length2, nav_leg_progress);
  //fflush(stdout);

  nav_segment_start = wp_start;
  nav_segment_end = wp_end;
  horizontal_mode = HORIZONTAL_MODE_ROUTE;
}

bool_t nav_approaching_from(uint8_t wp_idx, uint8_t from_idx) {
  int32_t dist_to_point;
  struct Int32Vect2 diff;
  VECT2_DIFF(diff, waypoints[wp_idx], ins.enu_pos);
  INT32_VECT2_RSHIFT(diff,diff,INT32_POS_FRAC);
  INT32_VECT2_NORM(dist_to_point, diff);
  //printf("dist %d | %d %d\n", dist_to_point,diff.x,diff.y);
  //fflush(stdout);
  if (dist_to_point < (ARRIVED_AT_WAYPOINT >> INT32_POS_FRAC)) return TRUE;
  if (from_idx > 0 && from_idx < NB_WAYPOINT) {
    struct Int32Vect2 from_diff;
    VECT2_DIFF(from_diff, waypoints[wp_idx],waypoints[from_idx]);
    INT32_VECT2_RSHIFT(from_diff,from_diff,INT32_POS_FRAC);
    return (diff.x * from_diff.x + diff.y * from_diff.y < 0);
  }
  else return FALSE;
}

static int32_t previous_ground_alt;

/** Reset the geographic reference to the current GPS fix */
unit_t nav_reset_reference( void ) {
  ins.ltp_initialised = FALSE;
  previous_ground_alt = ground_alt;
  ground_alt = ins.enu_pos.z;
  return 0;
}

/** Shift altitude of the waypoint according to a new ground altitude */
unit_t nav_update_waypoints_alt( void ) {
  uint8_t i;
  for(i = 0; i < NB_WAYPOINT; i++) {
    waypoints[i].z += ground_alt - previous_ground_alt;
  }
  return 0;
}

void nav_init_stage( void ) {
  INT32_VECT3_COPY(nav_last_point, ins.enu_pos);
  stage_time = 0;
  horizontal_mode = HORIZONTAL_MODE_WAYPOINT;
}

void nav_init_block(void) { 
  if (nav_block >= NB_BLOCK)
    nav_block=NB_BLOCK-1;
  nav_stage = 0;
  block_time = 0;
  InitStage();
}

void nav_goto_block(uint8_t b) {
  if (b != nav_block) { /* To avoid a loop in a the current block */
    last_block = nav_block;
    last_stage = nav_stage;
  }
  GotoBlock(b);
}

void nav_periodic_task_10Hz() {
  RunOnceEvery(10, { stage_time++;  block_time++; });

  /* from flight_plan.h */
  auto_nav();

  /* run carrot loop */
  booz2_nav_run();
}

void nav_move_waypoint(uint8_t wp_id, struct EnuCoor_i * new_pos) {
  if (wp_id < nb_waypoint) {
    INT32_VECT3_COPY(waypoints[wp_id],(*new_pos));
  }
}

void nav_home(void) {}

