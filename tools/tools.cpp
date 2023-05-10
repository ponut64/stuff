
uint16_t         swap_endian_ushort(uint16_t value)   { return ((((value) >> 8) & 0xff) | (((value) & 0xff) << 8));}
int16_t          swap_endian_sshort( int16_t value)   { return (((value >> 8)&0xFF) | ((value & 0xFF) << 8));}
uint32_t         swap_endian_uint(  uint32_t value)   { return ((((value) & 0xff000000) >> 24) | (((value) & 0x00ff0000) >>  8) | (((value) & 0x0000ff00) <<  8) | (((value) & 0x000000ff) << 24)); }
int32_t          swap_endian_sint(   int32_t value)   { return ((((value) & 0xff000000) >> 24) | (((value) & 0x00ff0000) >>  8) | (((value) & 0x0000ff00) <<  8) | (((value) & 0x000000ff) << 24)); }

void writeU8(ofstream * file, unsigned char data)
{
    file->write((char*)&data, sizeof(unsigned char));
}

void writeUint16(ofstream * file, uint16_t data)
{
    uint16_t buf = swap_endian_ushort(data);
    file->write((char*)&buf, sizeof(uint16_t));
}

void writeSint16(ofstream * file, int16_t data)
{
    int16_t buf = swap_endian_sshort(data);
    file->write((char*)&buf, sizeof(int16_t));
}

void writeUint32(ofstream * file, uint32_t data)
{
    uint32_t buf = swap_endian_uint(data);
    file->write((char*)&buf, sizeof(uint32_t));
}

void writeSint32(ofstream * file, int32_t data)
{
    int32_t buf = swap_endian_sint(data);
    file->write((char*)&buf, sizeof(int32_t));
}
