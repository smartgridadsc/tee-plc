//-----------------------------------------------------------------------------
// Copyright 2024 Illinois Advanced Research Center at Singapore
//
// This file is part of TEE-PLC.
//
// TEE-PLC is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// TEE-PLC is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with TEE-PLC.  If not, see <http://www.gnu.org/licenses/>.
//-----------------------------------------------------------------------------


#ifndef TA_SCAN_CYCLE_H
#define TA_SCAN_CYCLE_H

#define __DEBUG__ false
/*
 * This UUID is generated with uuidgen
 * the ITU-T UUID generator at http://www.itu.int/ITU-T/asn1/uuid.html
 */
#define TA_SCAN_CYCLE_UUID \
	{ 0x0bbd4f5b, 0xab7b, 0x4dcd, \
		{ 0xa1, 0xf5, 0x9b, 0x2e, 0xed, 0xaf, 0x0d, 0x1e} }

/* The function IDs implemented in this TA */
#define TA_SCAN_CYCLE_INIT		0
#define TA_SCAN_CYCLE_EXEC		1
#define TA_SCAN_CYCLE_EXIT		2
#endif /*TA_SCAN_CYCLE_H*/