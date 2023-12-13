//
// vim: ts=4 et
// Copyright (c) 2023 Petr Vanek, petr@fotoventus.cz
//
/// @file   lcs_info.h
/// @author Petr Vanek

#pragma once

#include <stdint.h>

struct LCSInfo {
    uint8_t hue;				///< hue value
	uint8_t intensity;			///< intensity value
    uint8_t command;			///< command as ordinal value	
	char id[15]; 				///< device ID as string with '\0'
};

