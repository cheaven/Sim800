#include "NekoDriver.h"
#include <windows.h>
extern "C" {
#include "ANSI/65c02.h"
}
#include <QtCore/QFile>


BYTE __stdcall NullRead (BYTE read);
void __stdcall NullWrite (BYTE write, BYTE value);

BYTE __stdcall StartTimer0 (BYTE read);
BYTE __stdcall StopTimer0 (BYTE read);
BYTE __stdcall StartTimer1 (BYTE read);
BYTE __stdcall StopTimer1 (BYTE read);

BYTE __stdcall ReadBank (BYTE read);
void __stdcall SwitchBank (BYTE write, BYTE value);
void __stdcall WriteLCDStartAddr (BYTE write, BYTE value);
void __stdcall WriteTimer01Control (BYTE write, BYTE value);

iofunction1 ioread[0x40]  = {
    ReadBank,       // $00
    NullRead,       // $01
    NullRead,       // $02
    NullRead,       // $03
    StopTimer0,     // $04
    StartTimer0,    // $05
    StopTimer1,     // $06
    StartTimer1,    // $07
    NullRead,       // $08
    NullRead,       // $09
    NullRead,       // $0A
    NullRead,       // $0B
    NullRead,       // $0C
    NullRead,       // $0D
    NullRead,       // $0E
    NullRead,       // $0F
    NullRead,       // $10
    NullRead,       // $11
    NullRead,       // $12
    NullRead,       // $13
    NullRead,       // $14
    NullRead,       // $15
    NullRead,       // $16
    NullRead,       // $17
    NullRead,       // $18
    NullRead,       // $19
    NullRead,       // $1A
    NullRead,       // $1B
    NullRead,       // $1C
    NullRead,       // $1D
    NullRead,       // $1E
    NullRead,       // $1F
    NullRead,       // $20
    NullRead,       // $21
    NullRead,       // $22
    NullRead,       // $23
    NullRead,       // $24
    NullRead,       // $25
    NullRead,       // $26
    NullRead,       // $27
    NullRead,       // $28
    NullRead,       // $29
    NullRead,       // $2A
    NullRead,       // $2B
    NullRead,       // $2C
    NullRead,       // $2D
    NullRead,       // $2E
    NullRead,       // $2F
    NullRead,       // $30
    NullRead,       // $31
    NullRead,       // $32
    NullRead,       // $33
    NullRead,       // $34
    NullRead,       // $35
    NullRead,       // $36
    NullRead,       // $37
    NullRead,       // $38
    NullRead,       // $39
    NullRead,       // $3A
    NullRead,       // $3B
    NullRead,       // $3C
    NullRead,       // $3D
    NullRead,       // $3E
    NullRead,       // $3F
};

iofunction2 iowrite[0x40] = {
    SwitchBank,     // $00
    NullWrite,      // $01
    NullWrite,      // $02
    NullWrite,      // $03
    NullWrite,      // $04
    NullWrite,      // $05
    WriteLCDStartAddr,  // $06
    NullWrite,      // $07
    NullWrite,      // $08
    NullWrite,      // $09
    NullWrite,      // $0A
    NullWrite,      // $0B
    WriteTimer01Control,    // $0C
    NullWrite,      // $0D
    NullWrite,      // $0E
    NullWrite,      // $0F
    NullWrite,      // $10
    NullWrite,      // $11
    NullWrite,      // $12
    NullWrite,      // $13
    NullWrite,      // $14
    NullWrite,      // $15
    NullWrite,      // $16
    NullWrite,      // $17
    NullWrite,      // $18
    NullWrite,      // $19
    NullWrite,      // $1A
    NullWrite,      // $1B
    NullWrite,      // $1C
    NullWrite,      // $1D
    NullWrite,      // $1E
    NullWrite,      // $1F
    NullWrite,      // $20
    NullWrite,      // $21
    NullWrite,      // $22
    NullWrite,      // $23
    NullWrite,      // $24
    NullWrite,      // $25
    NullWrite,      // $26
    NullWrite,      // $27
    NullWrite,      // $28
    NullWrite,      // $29
    NullWrite,      // $2A
    NullWrite,      // $2B
    NullWrite,      // $2C
    NullWrite,      // $2D
    NullWrite,      // $2E
    NullWrite,      // $2F
    NullWrite,      // $30
    NullWrite,      // $31
    NullWrite,      // $32
    NullWrite,      // $33
    NullWrite,      // $34
    NullWrite,      // $35
    NullWrite,      // $36
    NullWrite,      // $37
    NullWrite,      // $38
    NullWrite,      // $39
    NullWrite,      // $3A
    NullWrite,      // $3B
    NullWrite,      // $3C
    NullWrite,      // $3D
    NullWrite,      // $3E
    NullWrite,      // $3F
};

regsrec regs;
LPBYTE  mem          = NULL;
TCHAR   ROMfile[MAX_PATH] = TEXT("65c02.rom");
TCHAR   RAMfile[MAX_PATH] = TEXT("");


BYTE __stdcall NullRead (BYTE address) {
    qDebug("lee wanna read io, [%04x] -> %02x", address, mem[address]);
    return mem[address];
}

void __stdcall NullWrite(BYTE address, BYTE value) {
    qDebug("lee wanna write io, [%04x] (%02x) -> %02x", address, mem[address], value);
    mem[address] = value;
}

void MemDestroy () {
    VirtualFree(mem,0,MEM_RELEASE);
    mem      = NULL;
}

void MemInitialize () {
    mem = (LPBYTE)VirtualAlloc(NULL,0x10001,MEM_COMMIT,PAGE_READWRITE);

    if (!mem)	{
        MessageBox(GetDesktopWindow(),
            TEXT("The Simulator was unable to allocate the memory it ")
            TEXT("requires.  Further execution is not possible."),
            TITLE,
            MB_ICONSTOP | MB_SETFOREGROUND);
        ExitProcess(1);
    }

    //pre-fill rom area with 0xFF
    DWORD address = (0xFFFF - iopage);
    do *(mem + iopage + address) = (BYTE)(0xFF);
    while (address--);

    // READ THE FIRMWARE ROM INTO THE ROM IMAGE
    TCHAR filename[MAX_PATH];
    _tcscpy(filename,progdir);
    _tcscat(filename,ROMfile);

    HANDLE file = CreateFile(filename,
        GENERIC_READ,
        FILE_SHARE_READ,
        (LPSECURITY_ATTRIBUTES)NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,
        NULL);
    if (file == INVALID_HANDLE_VALUE) {
        //MessageBox(GetDesktopWindow(),
        //    TEXT("Unable to open the required firmware ROM data file.")
        //    TEXT("  Building an empty image in ROM filled with 0xFF's.")
        //    TEXT("  RESET will jump to 0x0300, IRQ to 0x0200, &")
        //    TEXT(" NMI to 0x0280.  A '????????.ROM' file is needed."),
        //    TITLE,
        //    MB_ICONSTOP | MB_SETFOREGROUND);
        qDebug("RESET will jump to 0x0418 in norflash page1.");

        *(mem + 0xFFFA) = (BYTE)(0x80);  // NMI Vector LowByte        0x0280
        *(mem + 0xFFFB) = (BYTE)(0x02);  // NMI Vector HighByte
        *(mem + 0xFFFC) = (BYTE)(0x18);  // Reset Vector LowByte      0x0300
        *(mem + 0xFFFD) = (BYTE)(0x40);  // Reset Vector HighByte
        *(mem + 0xFFFE) = (BYTE)(0x00);  // IRQ Vector LowByte        0x0200
        *(mem + 0xFFFF) = (BYTE)(0x02);  // IRQ Vector HighByte
    } else {
        DWORD bytesread;
        ReadFile(file,(mem+0x8000),0x8000,&bytesread,NULL);  //always read 32k
    }
    CloseHandle(file);    							   //memreset will fix ram

    MemReset();
}


void MemReset ()
{
    ZeroMemory(mem,iopage);

    // INITIALIZE THE CPU
    CpuInitialize(); // Read pc from reset vector
}

void __stdcall SwitchBank( BYTE write, BYTE value )
{
    mem[write] = value;
    qDebug("lee wanna switch to bank 0x%02x", value);
    theNekoDriver->SwitchNorBank(value);
}


BYTE __stdcall ReadBank( BYTE read )
{
    byte r = mem[00];
    qDebug("lee wanna read bank. current bank 0x%02x", r);
    return r;
}

bool timer0started = false;
bool timer1started = false;

BYTE __stdcall StartTimer0( BYTE read )
{
    qDebug("lee wanna start timer0");
    timer0started = true;
    return mem[02];
}

BYTE __stdcall StopTimer0( BYTE read )
{
    qDebug("lee wanna stop timer0");
    byte r = mem[02];
    mem[02] = 0;
    timer0started = false;
    return r;
}

BYTE __stdcall StartTimer1( BYTE read )
{
    qDebug("lee wanna start timer1");
    timer1started = true;
    return mem[03];
}

BYTE __stdcall StopTimer1( BYTE read )
{
    qDebug("lee wanna stop timer1");
    byte r = mem[03];
    mem[03] = 0;
    timer1started = false;
    return r;
}

unsigned short lcdbufaddr;

void __stdcall WriteLCDStartAddr( BYTE write, BYTE value )
{
    unsigned int t = ((mem[0x0C] & 0x3) << 12);
    t = t | (value << 4);
    qDebug("lee wanna change lcdbuf address to 0x%04x", t);
    mem[write] = value;
    lcdbufaddr = t;
}

void __stdcall WriteTimer01Control( BYTE write, BYTE value )
{
    unsigned int t = ((value & 0x3) << 12);
    t = t | (mem[0x06] << 4);
    qDebug("lee wanna change lcdbuf address to 0x%04x", t);
    qDebug("lee also wanna change timer settins to 0x%02x.", (value & 0xC));
    mem[write] = value;
    lcdbufaddr = t;
}


bool TNekoDriver::SwitchNorBank( int bank )
{
    memcpy(&mem[0x4000], fNorBuffer.data() + bank * 0x8000, 0x8000);
    return true;
}

bool TNekoDriver::LoadDemoNor(const QString& filename)
{
    QFile mariofile(filename);
    mariofile.open(QFile::ReadOnly);
    if (fNorBuffer.isEmpty()){
        fNorBuffer.resize(16 * 0x8000); // 32K * 16
    }
    int page = 1;
    while (mariofile.atEnd() == false) {
        mariofile.read(fNorBuffer.data() + 0x8000 * page + 0x4000, 0x4000);
        mariofile.read(fNorBuffer.data() + 0x8000 * page, 0x4000);
        page++;
    }
    //mariofile.read(fNorBuffer.data() + 0x8000 + 0x4000, 0x4000);
    //mariofile.read(fNorBuffer.data() + 0x8000, 0x4000);
    //mariofile.read(fNorBuffer.data() + 0x8000 + 0xC000, 0x4000);
    //mariofile.read(fNorBuffer.data() + 0x8000 + 0x8000, 0x4000);
    mariofile.close();
    return true;
}