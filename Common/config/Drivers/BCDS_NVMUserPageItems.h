/**
 * @file
 * @brief Defines needed NVM User Page items
 */

#ifndef BCDS_NVM_USERPAGEITEMS_H_
#define BCDS_NVM_USERPAGEITEMS_H_

/* note: this header file is project specific */

#include "BCDS_NVM.h"

/* Item Description : Value to write to NVM internal Buffer */
#define NVM_ITEM_BOOTLOADER_ENGAGE (struct NVM_Item_S) { \
   .endianness = NVM_ENDIANNESS_LITTLE, \
   .mode = NVM_MODE_RW, \
   .section_id = UINT8_C(1), \
   .length_byte = UINT32_C(1), \
   .start_address = UINT32_C(0xfe00000) \
}

/* Item Description : Value to write NVM content to internal Buffer */
#define NVM_ITEM_CONTENT_INDEX (struct NVM_Item_S) { \
   .endianness = NVM_ENDIANNESS_LITTLE, \
   .mode = NVM_MODE_RW, \
   .section_id = UINT8_C(1), \
   .length_byte = UINT32_C(1), \
   .start_address = UINT32_C(0xfe00001) \
}
/* Item Description : NVM WiFi MAC index value write to internal Buffer */
#define NVM_WIFI_MAC_INDEX   (struct NVM_Item_S) { \
    .endianness = NVM_ENDIANNESS_LITTLE, \
    .mode = NVM_MODE_RW, \
    .section_id = UINT8_C(1), \
    .length_byte = UINT32_C(6), \
    .start_address = UINT32_C(0xfe00002) \
}
/* Item Description : NVM BTLE MAC index value write to internal Buffer */
#define NVM_BTLE_MAC_INDEX (struct NVM_Item_S) { \
        .endianness = NVM_ENDIANNESS_LITTLE, \
        .mode = NVM_MODE_RW, \
        .section_id = UINT8_C(1), \
        .length_byte = UINT32_C(6), \
        .start_address = UINT32_C(0xfe00008) \
}
/* Item Description : NVM USB Serial number value write to internal Buffer */
#define NVM_USB_SERIAL_NUMBER (struct NVM_Item_S) { \
        .endianness = NVM_ENDIANNESS_LITTLE, \
        .mode = NVM_MODE_RW, \
        .section_id = UINT8_C(1), \
        .length_byte = UINT32_C(16), \
        .start_address = UINT32_C(0xfe0000E) \
}

/* Item Description: Next block number of firmware to be collected from server */
#define NVM_ITEM_ID_NEXT_BLOCK_NUMBER_SIZE UINT32_C(4)
#define NVM_ITEM_ID_NEXT_BLOCK_NUMBER (struct NVM_Item_S) { \
   .endianness = NVM_ENDIANNESS_LITTLE, \
   .mode = NVM_MODE_RW, \
   .section_id = UINT8_C(1), \
   .length_byte = UINT32_C(4), \
   .start_address = UINT32_C(0xfe0001E) \
}

/* Item Description: Firmware package url */
#define NVM_ITEM_ID_FIRMWARE_PACKAGE_URL_SIZE UINT32_C(120)
#define NVM_ITEM_ID_FIRMWARE_PACKAGE_URL (struct NVM_Item_S) { \
   .endianness = NVM_ENDIANNESS_NONE, \
   .mode = NVM_MODE_RW, \
   .section_id = UINT8_C(1), \
   .length_byte = UINT32_C(120), \
   .start_address = UINT32_C(0xfe00022) \
}

/* Item Description: Firmware download is in progress flag */
#define NVM_ITEM_ID_DOWNLOAD_IN_PROGRESS_SIZE UINT32_C(1)
#define NVM_ITEM_ID_DOWNLOAD_IN_PROGRESS (struct NVM_Item_S) { \
   .endianness = NVM_ENDIANNESS_LITTLE, \
   .mode = NVM_MODE_RW, \
   .section_id = UINT8_C(1), \
   .length_byte = UINT32_C(1), \
   .start_address = UINT32_C(0xfe0009A) \
}

/* Item Description: Fota current state */
#define NVM_ITEM_ID_FOTA_CURRENT_STATE_SIZE UINT32_C(4)
#define NVM_ITEM_ID_FOTA_CURRENT_STATE (struct NVM_Item_S) { \
   .endianness = NVM_ENDIANNESS_LITTLE, \
   .mode = NVM_MODE_RW, \
   .section_id = UINT8_C(1), \
   .length_byte = UINT32_C(4), \
   .start_address = UINT32_C(0xfe0009B) \
}

/* Item Description: FOTA result status flag  */
#define NVM_ITEM_ID_FOTA_CURRENT_RESULT_SIZE UINT32_C(1)
#define NVM_ITEM_ID_FOTA_CURRENT_RESULT (struct NVM_Item_S) { \
   .endianness = NVM_ENDIANNESS_LITTLE, \
   .mode = NVM_MODE_RW, \
   .section_id = UINT8_C(1), \
   .length_byte = UINT32_C(1), \
   .start_address = UINT32_C(0xfe0009F) \
}

/* Item Description: Is there a new firmware in the download partition */
#define NVM_ITEM_ID_IS_NEW_FW_SIZE UINT32_C(1)
#define NVM_ITEM_ID_IS_NEW_FW (struct NVM_Item_S) { \
   .endianness = NVM_ENDIANNESS_LITTLE, \
   .mode = NVM_MODE_RW, \
   .section_id = UINT8_C(1), \
   .length_byte = UINT32_C(1), \
   .start_address = UINT32_C(0xfe000A0) \
}

/* Item Description: To store the CRC of the firmware in download partition */
#define NVM_ITEM_ID_NEW_FW_CRC_SIZE UINT32_C(4)
#define NVM_ITEM_ID_NEW_FW_CRC (struct NVM_Item_S) { \
   .endianness = NVM_ENDIANNESS_LITTLE, \
   .mode = NVM_MODE_RW, \
   .section_id = UINT8_C(1), \
   .length_byte = UINT32_C(4), \
   .start_address = UINT32_C(0xfe000A1) \
}


/* Item Description : NVM user items total Buffer size */
#define NVM_SECTION_UserPage_BUFFER_SIZE UINT32_C(168)
#define NVM_SECTION_UserPage { \
    .section_id = UINT8_C(1), \
    .length_byte = UINT32_C(168), \
    .start_address = UINT32_C(0xfe00000), \
    .page_length_byte = UINT32_C(4096) \
}

#endif /* BCDS_NVM_USERPAGEITEMS_H_ */
