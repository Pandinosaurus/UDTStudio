
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include "OD_controller.h"

//==========test on OD_OBJECT_VAR =============
void testReadObjectVar()
{
    uint16_t *value;

    assert(OD_read(0x1017, 0x00, (void**)&value) == 6);
    assert(*value == 0x17);
}

void testReadObjectVarRO()
{
    uint8_t *value;

    assert(OD_read(0x1001, 0x00, (void**)&value) == 5);
    assert(*value == 0x1);
}

void testReadObjectVarNoObject()
{
    uint8_t *value;

    assert(OD_read(0x10, 0x00, (void**)&value) == -OD_ABORT_CODE_NO_OBJECT);
}

void testReadObjectVarNoSub()
{
	int32_t code;
    uint8_t *value;

    assert(OD_read(0x1001, 0x01, (void**)&value) == -OD_ABORT_CODE_NO_SUBINDEX);
}

//==========test on OD_OBJECT_RECORD ==========
void testReadObjectRecord()
{
    uint32_t *value;

    assert(OD_read(0x1018, 0x01, (void**)&value) == 7);
    assert(*value == 0x1);
}

void testReadObjectRecordNonConituous()
{
    uint32_t *value;

    assert(OD_read(0x1601, 0x08, (void**)&value) == 7);
    assert(*value == 0x8);
}

void testWriteObjectRecordNonConituous()
{
    uint32_t data = 4;
    uint32_t *value;

    assert(OD_write(0x1601, 0x08, (void*)&data, 4) == 7);
    assert(OD_read(0x1601, 0x08, (void**)&value) == 7);
    assert(*value == 4);
}

void testWriteObjectVar()
{
    uint32_t data = 4;
    uint16_t *value;

    assert(OD_write(0x1017, 0x00, (void*)&data, 2) == 6);
    assert(OD_read(0x1017, 0x00, (void**)&value) == 6);
    assert(*value == 4);
}

void testWriteObjectVarRO()
{
    uint32_t data = 4;
    uint8_t *value;

    assert(OD_write(0x1001, 0x00, (void*)&data, 1) == -OD_ABORT_CODE_READ_ONLY);
    assert(OD_read(0x1001, 0x00, (void**)&value) == 5);
    assert(*value == 0x1);
}

void testWriteObjectVarNoObject()
{
    uint32_t data = 4;
    uint8_t *value;

    assert(OD_write(0x10, 0x00, (void*)&data, 1) == -OD_ABORT_CODE_NO_OBJECT);
}

void testWriteObjectVarNoSub()
{
    uint32_t data = 4;
    uint8_t *value;

    assert(OD_write(0x1017, 0x06, (void*)&data, 1) == -OD_ABORT_CODE_NO_SUBINDEX);
    assert(OD_write(0x1601, 0x06, (void*)&data, 1) == -OD_ABORT_CODE_NO_SUBINDEX);
}

//============= test on array ==============
void testWriteArray()
{
    uint32_t data = 4;
    uint8_t *value;

    assert(OD_write(0x6047, 0x1, (void*)&data, 4) == 7);
    assert(OD_read(0x6047, 0x01, (void**)&value) == 7);
    assert(*value == 4);
}

void testWriteArrayRO()
{
    int16_t data = 4;
    uint8_t *value;

    assert(OD_write(0x604B, 0x2, (void*)&data, 2) == -OD_ABORT_CODE_READ_ONLY);
    assert(OD_read(0x604B, 0x02, (void**)&value) == 3);
    assert(*value == 1);
}

void testWriteArrayNoObject()
{
    int16_t data = 4;
    uint8_t *value;

    assert(OD_write(0x604, 0x0, (void*)&data, 2) == -OD_ABORT_CODE_NO_OBJECT);
    assert(OD_read(0x604, 0x0, (void**)&value) == -OD_ABORT_CODE_NO_OBJECT);
}

void testWriteArrayNoSub()
{
    int16_t data = 4;
    uint8_t *value;

    assert(OD_write(0x6047, 0x7, (void*)&data, 2) == -OD_ABORT_CODE_NO_SUBINDEX);
    assert(OD_read(0x6047, 0x7, (void**)&value) == -OD_ABORT_CODE_NO_SUBINDEX);

    assert(OD_write(0x1017, 0x7, (void*)&data, 2) == -OD_ABORT_CODE_NO_SUBINDEX);
    assert(OD_read(0x1017, 0x7, (void**)&value) == -OD_ABORT_CODE_NO_SUBINDEX);
}

void testWriteBadSize()
{
    int16_t data = 4;
    uint8_t *value;

    assert(OD_write(0x6047, 0x4, (void*)&data, 2) == -OD_ABORT_CODE_LENGTH_DOESNT_MATCH);
    assert(OD_write(0x1017, 0x0, (void*)&data, 4) == -OD_ABORT_CODE_LENGTH_DOESNT_MATCH);
}

//=========test on OD_TYPE_VISIBLE_STRING=========
void testReadVisibleString()
{
    vstring_t*  value;

    assert(OD_read(0x1008, 0x00, (void**)&value) == 9);
    assert(strcmp(*value, "test") == 0);
}

int main()
{
	OD_reset();

	testReadObjectVar();
    testReadObjectVarRO();
    testReadObjectVarNoObject();
    testReadObjectVarNoSub();
    testReadObjectRecord();
    testReadObjectRecordNonConituous();
	testReadVisibleString();
    testWriteObjectRecordNonConituous();
    testWriteObjectVar();
    testWriteObjectVarRO();
    testWriteObjectVarNoObject();
    testWriteObjectVarNoSub();
    testWriteArray();
    testWriteArrayRO();
    testWriteArrayNoObject();
    testWriteArrayNoSub();
    testWriteBadSize();

	return 0;
}
