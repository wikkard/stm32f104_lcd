#include "app_config.h"

#if APP_USE_LVGL

#include "touch_calibration_store.h"
#include "touch_calibration_session.h"
#include "main.h"

extern const uint8_t __calibration_start__;
extern const uint8_t __calibration_end__;

#define RECORD_MAGIC 0x5443414CUL
#define RECORD_FORMAT ((52UL << 16) | 1UL)
/* Changes to the LCD orientation/geometry must change this compatibility tag. */
#define RECORD_PROFILE 0x00680001UL
#define RECORD_GEOMETRY ((320UL << 16) | 240UL)
#define RECORD_COMMIT 0xFFFFA55AUL
#define RECORD_WORDS 13U
#define RECORD_CRC_WORD 11U
#define RECORD_COMMIT_WORD 12U

typedef struct
{
    uint32_t words[RECORD_WORDS];
} calibration_record_t;

_Static_assert(sizeof(calibration_record_t) == 52, "Calibration record layout changed");
_Static_assert(FLASH_PAGE_SIZE == 2048, "Calibration requires 2 KiB flash pages");

static uintptr_t slot_address(uint32_t slot)
{
    return (uintptr_t)&__calibration_start__ + slot * FLASH_PAGE_SIZE;
}

static bool layout_valid(void)
{
    uintptr_t start = (uintptr_t)&__calibration_start__;
    uintptr_t end = (uintptr_t)&__calibration_end__;
    return start % FLASH_PAGE_SIZE == 0 && end - start == 2U * FLASH_PAGE_SIZE &&
           start > FLASH_BANK1_END && end == FLASH_BANK2_END + 1U &&
           *(const uint16_t *)FLASH_SIZE_DATA_REGISTER == 1024U;
}

static void read_record(uint32_t slot, calibration_record_t *record)
{
    const volatile uint32_t *source = (const volatile uint32_t *)slot_address(slot);
    for(uint32_t i = 0; i < RECORD_WORDS; ++i)
    {
        record->words[i] = source[i];
    }
}

static uint32_t record_crc(const calibration_record_t *record)
{
    uint32_t crc = 0xFFFFFFFFU;
    for(uint32_t i = 0; i < RECORD_CRC_WORD * 4U; ++i)
    {
        uint32_t byte = (record->words[i / 4U] >> ((i % 4U) * 8U)) & 0xFFU;
        crc ^= byte;
        for(uint32_t bit = 0; bit < 8; ++bit)
        {
            crc = (crc >> 1) ^ ((crc & 1U) ? 0xEDB88320U : 0U);
        }
    }
    return ~crc;
}

static bool decode_record(const calibration_record_t *record, touch_point_t raw[3],
                           touch_calibration_t *calibration)
{
    if(record->words[0] != RECORD_MAGIC || record->words[1] != RECORD_FORMAT ||
       record->words[3] != RECORD_PROFILE || record->words[4] != RECORD_GEOMETRY ||
       record->words[RECORD_COMMIT_WORD] != RECORD_COMMIT ||
       record->words[RECORD_CRC_WORD] != record_crc(record))
    {
        return false;
    }
    for(uint32_t i = 0; i < 3; ++i)
    {
        if(record->words[5U + i * 2U] > 4095 || record->words[6U + i * 2U] > 4095)
        {
            return false;
        }
        raw[i] = (touch_point_t){(int32_t)record->words[5U + i * 2U],
                                 (int32_t)record->words[6U + i * 2U]};
    }
    return touch_calibration_fit(calibration, raw, touch_calibration_targets);
}

static int32_t newest_slot(calibration_record_t records[2])
{
    touch_point_t raw[3];
    touch_calibration_t calibration;
    read_record(0, &records[0]);
    read_record(1, &records[1]);
    bool a = decode_record(&records[0], raw, &calibration);
    bool b = decode_record(&records[1], raw, &calibration);
    if(!a) return b ? 1 : -1;
    if(!b) return 0;
    uint32_t delta = records[1].words[2] - records[0].words[2];
    return delta != 0 && delta < 0x80000000U ? 1 : 0;
}

touch_store_result_t touch_calibration_store_load(touch_point_t raw[3],
                                                  touch_calibration_t *calibration)
{
    *calibration = (touch_calibration_t){0};
    if(!layout_valid()) return TOUCH_STORE_LAYOUT;
    calibration_record_t records[2];
    int32_t slot = newest_slot(records);
    if(slot >= 0)
    {
        return decode_record(&records[slot], raw, calibration) ? TOUCH_STORE_OK :
                                                                TOUCH_STORE_INVALID;
    }
    for(uint32_t slot_index = 0; slot_index < 2; ++slot_index)
    {
        for(uint32_t i = 0; i < RECORD_WORDS; ++i)
        {
            if(records[slot_index].words[i] != 0xFFFFFFFFU) return TOUCH_STORE_INVALID;
        }
    }
    return TOUCH_STORE_EMPTY;
}

touch_store_result_t touch_calibration_store_save(const touch_point_t raw[3])
{
    if(!layout_valid()) return TOUCH_STORE_LAYOUT;
    touch_calibration_t calibration;
    if(!touch_calibration_fit(&calibration, raw, touch_calibration_targets))
    {
        return TOUCH_STORE_INVALID;
    }
    calibration_record_t records[2];
    int32_t newest = newest_slot(records);
    uint32_t target = newest == 0 ? 1U : 0U;
    calibration_record_t record = {.words = {
        RECORD_MAGIC, RECORD_FORMAT, 0, RECORD_PROFILE, RECORD_GEOMETRY
    }};
    record.words[2] = newest < 0 ? 0U : records[newest].words[2] + 1U;
    for(uint32_t i = 0; i < 3; ++i)
    {
        record.words[5U + i * 2U] = (uint32_t)raw[i].x;
        record.words[6U + i * 2U] = (uint32_t)raw[i].y;
    }
    record.words[RECORD_CRC_WORD] = record_crc(&record);
    record.words[RECORD_COMMIT_WORD] = 0xFFFFFFFFU;

    touch_store_result_t result = TOUCH_STORE_OK;
    if(HAL_FLASH_Unlock() != HAL_OK)
    {
        result = TOUCH_STORE_UNLOCK;
    }
    if(result == TOUCH_STORE_OK)
    {
        __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP_BANK2 | FLASH_FLAG_PGERR_BANK2 |
                               FLASH_FLAG_WRPERR_BANK2);
        FLASH_EraseInitTypeDef erase = {
            .TypeErase = FLASH_TYPEERASE_PAGES,
            .Banks = FLASH_BANK_2,
            .PageAddress = (uint32_t)slot_address(target),
            .NbPages = 1
        };
        uint32_t page_error = 0xFFFFFFFFU;
        if(HAL_FLASHEx_Erase(&erase, &page_error) != HAL_OK || page_error != 0xFFFFFFFFU)
        {
            result = TOUCH_STORE_ERASE;
        }
    }
    for(uint32_t halfword = 0; result == TOUCH_STORE_OK &&
        halfword < RECORD_COMMIT_WORD * 2U; ++halfword)
    {
        uint16_t value = (uint16_t)(record.words[halfword / 2U] >>
                                    ((halfword % 2U) * 16U));
        if(HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD,
                             (uint32_t)slot_address(target) + halfword * 2U, value) != HAL_OK)
        {
            result = TOUCH_STORE_PROGRAM;
        }
    }
    calibration_record_t verify;
    if(result == TOUCH_STORE_OK)
    {
        read_record(target, &verify);
        for(uint32_t i = 0; i < RECORD_WORDS; ++i)
        {
            if(verify.words[i] != record.words[i]) result = TOUCH_STORE_VERIFY;
        }
    }
    if(result == TOUCH_STORE_OK &&
       HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD,
                          (uint32_t)slot_address(target) + RECORD_COMMIT_WORD * 4U,
                          (uint16_t)RECORD_COMMIT) != HAL_OK)
    {
        result = TOUCH_STORE_PROGRAM;
    }
    if(result == TOUCH_STORE_OK)
    {
        read_record(target, &verify);
        touch_point_t decoded[3];
        if(!decode_record(&verify, decoded, &calibration)) result = TOUCH_STORE_VERIFY;
    }
    if(HAL_FLASH_Lock() != HAL_OK) result = TOUCH_STORE_LOCK;
    return result;
}

const char *touch_calibration_store_message(touch_store_result_t result)
{
    switch(result)
    {
        case TOUCH_STORE_OK: return "Saved calibration";
        case TOUCH_STORE_EMPTY: return "First startup - calibrate touch";
        case TOUCH_STORE_INVALID: return "Invalid calibration - recalibrate";
        case TOUCH_STORE_LAYOUT: return "Flash layout/device mismatch";
        case TOUCH_STORE_UNLOCK: return "Flash unlock failed";
        case TOUCH_STORE_ERASE: return "Flash erase failed";
        case TOUCH_STORE_PROGRAM: return "Flash programming failed";
        case TOUCH_STORE_VERIFY: return "Flash verification failed";
        case TOUCH_STORE_LOCK: return "Flash lock failed";
        default: return "Unknown calibration storage error";
    }
}

#endif
