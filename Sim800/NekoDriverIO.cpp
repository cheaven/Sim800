#include "NekoDriver.h"
extern "C" {
#ifdef HANDYPSP
#include "ANSI/w65c02.h"
#else
#include "ANSI/65C02.h"
#endif
}
#include "CC800IOName.h"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>


bool timer0started = false;
bool timer1started = false;

// WQXSIM
bool timer0waveoutstart = false;
int prevtimer0value = 0;
unsigned short gThreadFlags;
unsigned char* gGeneralCtrlPtr;
unsigned short mayGenralnClockCtrlValue;

BYTE __iocallconv Read05StartTimer0( BYTE ) // 05
{
    // SPDC1016
    qDebug("ggv wanna start timer0");
    timer0started = true;
    if (fixedram0000[io02_timer0_val] == 0x3F) {
        //gTimer0WaveoutStarted = 1;
        //mayTimer0Var1 = 0;
        //maypTimer0VarA8 = (int)&unk_4586A8;
        //mayTimer0Var2 = 0;
        //mayIO2345Var1 = 0;
        //ResetWaveout(&pwh);
        //OpenWaveout((DWORD_PTR)&pwh, 0x1F40u);
        timer0waveoutstart = true;
    }
    prevtimer0value = fixedram0000[io02_timer0_val];
    return fixedram0000[io05_clock_ctrl]; // follow rulz by GGV
}

BYTE __iocallconv Read04StopTimer0( BYTE ) // 04
{
    // SPDC1016
    qDebug("ggv wanna stop timer0");
    //byte r = fixedram0000[io02_timer0_val];
    //fixedram0000[io02_timer0_val] = 0;
    //if ( gTimer0WaveoutStarted )
    //{
    //    if ( mayIO2345Var1 > 0 )
    //        byte_4603B8[mayIO45Var3x] = 1;
    //    CloseWaveout(&pwh);
    //    gTimer0WaveoutStarted = 0;
    //}
    timer0started = false;
    if (timer0waveoutstart) {
        timer0waveoutstart = false;
    }
    return fixedram0000[io04_general_ctrl];
}

BYTE __iocallconv Read07StartTimer1( BYTE ) // 07
{
    // SPDC1016
    //qDebug("ggv wanna start timer1");
    //timer1started = true;
    gThreadFlags &= 0xFFFDu; // Remove 0x02
    return fixedram0000[io07_port_config];
}

BYTE __iocallconv Read06StopTimer1( BYTE ) // 06
{
    // Stop timer1, and return time1 value
    // SPDC1016
    //qDebug("ggv wanna stop timer1");
    //byte r = fixedram0000[io03_timer1_val];
    //fixedram0000[io03_timer1_val] = 0;
    //timer1started = false;
    gThreadFlags |= 0x02; // Add 0x02
    gGeneralCtrlPtr = &fixedram0000[io04_general_ctrl];
    mayGenralnClockCtrlValue = *gGeneralCtrlPtr;
    return fixedram0000[io06_lcd_config];
}

bool lcdoffshift0flag = false;

void __iocallconv Write05ClockCtrl( BYTE write, BYTE value )
{
    // FROM WQXSIM
    // SPDC1016
    if (fixedram0000[io05_clock_ctrl] & 0x8) {
        // old bit3, LCDON
        // Previous LCD on
        if ((value & 0xF) == 0) {
            // new value bit0~3 is 0
            // LCD off, lcd shift clock select bit0~3 is 0
            lcdoffshift0flag = true;
        }
    }
    fixedram0000[io05_clock_ctrl] = value;
    (void)write;
}

unsigned short lcdbuffaddr;

void __iocallconv Write06LCDStartAddr( BYTE write, BYTE value ) // 06
{
    unsigned int t = ((fixedram0000[io0C_lcd_config] & 0x3) << 12);
    t = t | (value << 4);
    qDebug("ggv wanna change lcdbuf address to 0x%04x", t);
    fixedram0000[io06_lcd_config] = value;
    lcdbuffaddr = t;
    (void)write;
    // SPDC1016
    // don't know how wqxsim works.
    fixedram0000[io09_port1_data] &= 0xFEu; // remove bit0 of port1 (keypad)
}

void __iocallconv WriteTimer01Control( BYTE write, BYTE value ) // 0C
{
    unsigned int t = ((value & 0x3) << 12); // lc12~lc13
    t = t | (fixedram0000[io06_lcd_config] << 4); // lc4~lc11
    qDebug("ggv wanna change lcdbuf address to 0x%04x", t);
    qDebug("ggv also wanna change timer settings to 0x%02x.", (value & 0xC));
    fixedram0000[io0C_lcd_config] = value;
    lcdbuffaddr = t;
    (void)write;
}

void __iocallconv Write20JG( BYTE write, BYTE value )
{
    // SPDC1016

    if (value == 0x80u) {
        //memset(dword_44B988, 0, 0x20u);
        //gFixedRAM1_b20 = 0;           // mem[20] change from 80 to 00
        //LOBYTE(mayIO23Index1) = 0;
        //mayIO20Flag1 = 1;
        fixedram0000[io20_JG] = 0;
    } else {
        fixedram0000[io20_JG] = value;
    }
    (void)write;
}


void __iocallconv Write23Unknow( BYTE write, BYTE value )
{
    // SPDC1023
    // io23 unknown
    //currentdata = tmpAXYValue;    // current mem[23] value
    //if ( tmpAXYValue == 0xC2u )
    //{
    //    // mayIO23Index used in some waveplay routine
    //    dword_4603D4[(unsigned __int8)mayIO23Index1] = gFixedRAM1_b22;
    //}
    //else
    //{
    //    if ( tmpAXYValue == 0xC4u )
    //    {
    //        // for PC1000?
    //        dword_44EA1C[(unsigned __int8)mayIO23Index1] = gFixedRAM1_b22;
    //        LOBYTE(mayIO23Index1) = mayIO23Index1 + 1;
    //    }
    //}
    //if ( gTimer0WaveoutStarted )
    //{
    //    *(_BYTE *)maypTimer0VarA8 = currentdata;
    //    v2 = mayTimer0Var1 + 1;
    //    ++maypTimer0VarA8;
    //    overflowed = mayIO2345Var1 == 7999;
    //    ++mayTimer0Var1;
    //    ++mayIO2345Var1;
    //    if ( overflowed )
    //    {
    //        byte_4603B8[mayIO45Var3x] = 1;
    //        if ( v2 == 8000 )
    //            WriteWaveout(&pwh);
    //        mayIO2345Var1 = 0;
    //    }
    //    destaddr = mayDestAddr;
    //}
    //if ( tmpAXYValue == 0x80u )
    //{
    //    gFixedRAM1_b20 = 0x80u;
    //    mayIO20Flag1 = 0;
    //    if ( (_BYTE)mayIO23Index1 > 0u )
    //    {
    //        if ( !gTimer0WaveoutStarted )
    //        {
    //            GenerateAndPlayJGWav();
    //            destaddr = mayDestAddr;
    //            LOBYTE(mayIO23Index1) = 0;
    //        }
    //    }
    //}
    if (value == 0xC2u) {

    } else if (value == 0xC4) {

    }
    if (timer0waveoutstart) {

    }
    if (value == 0x80u) {
        if (!timer0waveoutstart) {

        }
    }
    fixedram0000[io23_unknow] = value;
    (void)write;
}

void __iocallconv Write02Timer0Value( BYTE write, BYTE value )
{
    // SPDC1016
    if (timer0started) {
        prevtimer0value = value;
    }
    fixedram0000[io02_timer0_val] = value;
    (void)write;
}


//////////////////////////////////////////////////////////////////////////
// Keypad registers
//////////////////////////////////////////////////////////////////////////
unsigned /*char*/ keypadmatrix[8][8] = {0,};

void UpdateKeypadRegisters()
{
    // TODO: 2pass check
    //qDebug("old [0015]:%02x [0009]:%02x [0008]:%02x", mem[0x15], mem[0x9], mem[0x8]);
    //int up = 0, down = 0;
    unsigned char port1control = fixedram0000[io15_port1_dir];
    unsigned char port0control = fixedram0000[io0F_port0_dir] & 0xF0; // b4~b7
    unsigned char port1controlbit = 1; // aka, y control bit
    unsigned char tmpdest0 = 0, tmpdest1 = 0;
    unsigned char port1data = fixedram0000[io09_port1_data], port0data = fixedram0000[io08_port0_data];
    for (int y = 0; y < 8; y++) {
        // y = Port10~Port17
        bool ysend = ((port1control & port1controlbit) != 0);
        unsigned char xbit = 1;
        for (int x = 0; x < 8; x++) {
            // x = Port00~Port07
            unsigned char port0controlbit;
            if (x < 2) {
                // 0, 1 = b4 b5
                port0controlbit = xbit << 4;
            } else if (x < 4) {
                // 2, 3 = b6
                port0controlbit = 0x40;
            } else {
                // 4, 5, 6, 7 = b7
                port0controlbit = 0x80u;
            }
            if (y < 2 && (port1data == 0x02 || port1data == 0x01)) {
                // Emulator rulz, only for port1 is single 0x02 0x01
                // TODO: invert when y < 2 (row 6,7)
                // 0,2 is both high
                if (ysend) {
                    // port1y-> port0x
                    // port1y is send but only set bit to high when port0 xbit is receive too
                    if ((keypadmatrix[y][x] != 1) && ((port1data & port1controlbit) != 0) && ((port0control & port0controlbit) == 0)) {
                        tmpdest0 |= xbit;
                    }
                } else {
                    // port0x -> port1y
                    // port1y should be receive, only set bit to high when port0 xbit is send
                    if ((keypadmatrix[y][x] != 1) && ((port0data & xbit) != 0) && ((port0control & port0controlbit) != 0)) {
                        tmpdest1 |= xbit;
                    }
                }
            } else if (keypadmatrix[y][x] != 2) {
                if (ysend) {
                    // port1y-> port0x
                    // port1y is send but only set bit to high when port0 xbit is receive too
                    if ((keypadmatrix[y][x]) && ((port1data & port1controlbit) != 0) && ((port0control & port0controlbit) == 0)) {
                        tmpdest0 |= xbit;
                    }
                } else {
                    // port0x -> port1y
                    // port1y should be receive, only set bit to high when port0 xbit is send
                    if ((keypadmatrix[y][x]) && ((port0data & xbit) != 0) && ((port0control & port0controlbit) != 0)) {
                        tmpdest1 |= xbit;
                    }
                }
            }
            xbit = xbit << 1;
        }
        port1controlbit = port1controlbit << 1;
    }
    if (port1control != 0xFFu) {
        // port1 should clean some bits
        // using port1control as port1mask
        // sometimes port10,11 should clean here 
        port1data &= port1control; // pre set receive bits to 0
    }
    if (port1control != 0xF0u) {
        // clean port0
        // calculate port0 mask
        // in most case port0 will be set to 0
        unsigned char port0mask = (port0control >> 4) & 0x3; // bit4->0 bit5->1
        if (port0control & 0x40) {
            // bit6->2,3
            port0mask |= 0x0C; // 00001100
        }
        if (port0control & 0x80u) {
            // bit7->4,5,6,7
            port0mask |= 0xF0u; // 11110000
        }
        port0data &= port0mask;
    }
    port0data |= tmpdest0;
    port1data |= tmpdest1;
    if (fixedram0000[io09_port1_data] != port1data || fixedram0000[io08_port0_data] != port0data) {
        qDebug("old [0015]:%02x [0009]:%02x [0008]:%02x", fixedram0000[io15_port1_dir], fixedram0000[io09_port1_data], fixedram0000[io08_port0_data]);
        qDebug("new [0015]:%02x [0009]:%02x [0008]:%02x", fixedram0000[io15_port1_dir], port1data, port0data);
    }
    fixedram0000[io09_port1_data] = port1data;
    fixedram0000[io08_port0_data] = port0data;
}

BYTE __iocallconv ReadPort0( BYTE read )
{
    UpdateKeypadRegisters();
    //qDebug("ggv wanna read keypad port0, [%04x] -> %02x", read, mem[read]);
    return fixedram0000[io08_port0_data];
    (void)read;
}

BYTE __iocallconv ReadPort1( BYTE read )
{
    UpdateKeypadRegisters();
    //qDebug("ggv wanna read keypad port1, [%04x] -> %02x", read, mem[read]);
    return fixedram0000[io09_port1_data];
    (void)read;
}

void __iocallconv Write08Port0( BYTE write, BYTE value )
{
    //qDebug("ggv wanna write keypad port0, [%04x] (%02x) -> %02x", write, mem[write], value);
    fixedram0000[io08_port0_data] = value;
    // cosply simulator
    //// dest memory is already set to tmpAXYValue before writeio08
    //// simulator only support 09 output, 08 receive
    //newvalue = tmpAXYValue;
    //// P00~P07
    //// bit0~bit7
    //io0b = io0B_port3;
    //io0bvalue1 = gFixedRAM0[io0B_port3] | 1;    // set bit0 (LCD DOT OUT DIRECTION BIT1)  of lcd config
    //ishotkey = keypadmatrix1[6] == tmpAXYValue; // ram[io08] value is same to one of Hotkey in row6
    //gFixedRAM0[io0B_port3] = io0bvalue1;        // Add LCDIR1 |= 0x1;
    //if ( ishotkey || !newvalue || keypadmatrix1[7] == 0xFB )
    //    // newvalue fit to some hotkey in row6
    //    // or newvalue is 0 (all colume is 0)
    //    // or row7 == FB (ON/OFF)
    //    gFixedRAM0[io0b] = io0bvalue1 & 0xFE;   // Remove LCDIR1
    unsigned char xbit = 1;
    unsigned char row6data = 0, row7data = 0;
    for (int x = 0; x < 8; x++) {
        if ((keypadmatrix[1][x] != 1)) {
            row6data |= xbit;
        }
        if ((keypadmatrix[0][x] != 1)) {
            row7data |= xbit;
        }
        xbit = xbit << 1;
    }
    // workaround?
    if (row6data == 0xFF) {
        row6data = 0;
    }
    if (row7data == 0xFF) {
        row7data = 0;
    }
    if (row6data == value || value == 0 || row7data == 0xFBu) {
        // newvalue fit to some hotkey in row6
        // or newvalue is 0 (all colume is 0)
        // or row7 == FB (ON/OFF)
        fixedram0000[io0B_lcd_ctrl] &= 0xFEu; // Remove LCDIR1
    } else {
        fixedram0000[io0B_lcd_ctrl] |= 0x01; // Add LCDIR1 |= 0x1
    }
    UpdateKeypadRegisters();
    (void)write;
}

void __iocallconv Write09Port1( BYTE write, BYTE value )
{
    //qDebug("ggv wanna write keypad port1, [%04x] (%02x) -> %02x", write, mem[write], value);
    fixedram0000[io09_port1_data] = value;
    // cosply simulator
    unsigned char xbit = 1;
    unsigned char row6data = 0, row7data = 0;
    for (int x = 0; x < 8; x++) {
        if ((keypadmatrix[1][x] != 1)) {
            row6data |= xbit;
        }
        if ((keypadmatrix[0][x] != 1)) {
            row7data |= xbit;
        }
        xbit = xbit << 1;
    }
    // workaround
    if (row6data == 0xFF) {
        row6data = 0;
    }
    if (row7data == 0xFF) {
        row7data = 0;
    }
    unsigned char port0bit01 = fixedram0000[io08_port0_data] & 3;
    if (value == 0) {
        //case 0u:
        //    // none of P10~P17 is set.
        //    io0b_2 = io0B_port3;
        //    port0bit01_ = gFixedRAM0[io08_port0_real] & 3;// 00 01 10 11
        //    row6iszero = keypadmatrix1[6] == 0;     // no hotkey
        //    row7data = keypadmatrix1[7];        // ON/OFF
        //    gFixedRAM0[io0B_port3] = port0bit01_;// remove b2~b7
        //    if ( !row6iszero || row7data )
        //        // have hotkey, or have on/off
        //        gFixedRAM0[io0b_2] = port0bit01_ ^ 3;// 00 -> 11 01 -> 10 10 -> 01 11 -> 00
        //    if ( row7data == 0xFD )
        //        // row7 is record/play
        //        gFixedRAM0[io0b_2] &= 0xFEu;    // Remove LCDIR1
        //    break;
        fixedram0000[io0B_lcd_ctrl] = port0bit01; // remove b2~b7
        if ((row6data != 0xFFu) || (row7data != 0xFFu)) {
            // hotkey or on/off
            fixedram0000[io0B_lcd_ctrl] = port0bit01 ^ 0x3; // remove b2~b7, reverse b0,b1
        }
        if (row7data == 0xFDu) {
            // record/play
            fixedram0000[io0B_lcd_ctrl] &= 0xFEu;
        }
    }
    if ((value == 0xFDu) || (value == 0xFEu)) {
        //case 0xFDu:
        //case 0xFEu:
        //    // ~P16 ~P17
        //    io0b_1 = io0B_port3;
        //    port0bit01 = gFixedRAM0[io08_port0_real] & 3;// 00 01 10 11
        //    issame = keypadmatrix1[7] == tmpAXYValue;
        //    gFixedRAM0[io0B_port3] = port0bit01;// remove b2~b7
        //    if ( issame )
        //        // row7 is same as [09]
        //        gFixedRAM0[io0b_1] = port0bit01 ^ 3;// remove b2~b7, reverse b0,b1
        //    break;
        fixedram0000[io0B_lcd_ctrl] = port0bit01; // remove b2~b7
        if (row7data == value) {
            // row7 is same as [09]
            fixedram0000[io0B_lcd_ctrl] = port0bit01 ^ 0x3; // remove b2~b7, reverse b0,b1
        }
    }
    if (value == 0x03) {
        //case 3u:
        //    // both P11 P10, used for p00~p07 send and all 1, p10-p11 send and all 1
        //    io0b = io0B_port3;
        //    io08valuebit01 = gFixedRAM0[io08_port0_real] & 3;
        //    gFixedRAM0[io0B_port3] = io08valuebit01;// remove b2~b7
        //    if ( row7data_ == 0xFB )
        //        gFixedRAM0[io0b] = io08valuebit01 ^ 3;// remove b2~b7, reverse b0,b1
        //    goto LABEL_19;
        fixedram0000[io0B_lcd_ctrl] = port0bit01; // remove b2~b7
        if (row7data == 0xFB) {
            // on/off
            fixedram0000[io0B_lcd_ctrl] = port0bit01 ^ 0x3; // remove b2~b7, reverse b0,b1
        }
    }
    // FIXME: 02, 01 Emulator rulz
    if (value == 0x02) {
        fixedram0000[io08_port0_data] = row6data;
    }
    if (value == 0x01) {
        fixedram0000[io08_port0_data] = row7data;
    }
    if (((value != 0xFDu) && (value != 0xFEu) && (value != 0x00) && (value != 0x02) && (value != 0x01) && (value != 0x03)) || ((value == 0x03) && (fixedram0000[io15_port1_dir] == 0xFCu))) {
        UpdateKeypadRegisters();
    }
    (void)write;
}

void __iocallconv ControlPort1( BYTE write, BYTE value )
{
    //qDebug("ggv wanna config keypad port1, [%04x] (%02x) -> %02x", write, mem[write], value);
    fixedram0000[io15_port1_dir] = value;
    UpdateKeypadRegisters();
    (void)write;
}
