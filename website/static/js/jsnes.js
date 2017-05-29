/*
JSNES, based on Jamie Sanders' vNES
Copyright (C) 2010 Ben Firshman

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

var JSNES = function(opts) {
    this.opts = {
        ui: JSNES.DummyUI,
        swfPath: 'lib/',

        preferredFrameRate: 60,
        fpsInterval: 500, // Time between updating FPS in ms
        showDisplay: true,

        emulateSound: true,
        sampleRate: 44100, // Sound sample rate in hz

        CPU_FREQ_NTSC: 1789772.5, //1789772.72727272d;
        CPU_FREQ_PAL: 1773447.4
    };
    if (typeof opts != 'undefined') {
        var key;
        for (key in this.opts) {
            if (typeof opts[key] != 'undefined') {
                this.opts[key] = opts[key];
            }
        }
    }

    this.frameTime = 1000 / this.opts.preferredFrameRate;

    this.ui = new this.opts.ui(this);
    this.cpu = new JSNES.CPU(this);
    this.ppu = new JSNES.PPU(this);
    this.papu = new JSNES.PAPU(this);
    this.mmap = null; // set in loadRom()
    this.keyboard = new JSNES.Keyboard();

    this.ui.updateStatus("Ready to load a ROM.");
};

JSNES.VERSION = "<%= version %>";

JSNES.prototype = {
    isRunning: false,
    fpsFrameCount: 0,
    romData: null,

    // Resets the system
    reset: function() {
        if (this.mmap !== null) {
            this.mmap.reset();
        }

        this.cpu.reset();
        this.ppu.reset();
        this.papu.reset();
    },

    start: function() {
        var self = this;

        if (this.rom !== null && this.rom.valid) {
            if (!this.isRunning) {
                this.isRunning = true;

                this.frameInterval = setInterval(function() {
                    self.frame();
                }, this.frameTime);
                this.resetFps();
                this.printFps();
                this.fpsInterval = setInterval(function() {
                    self.printFps();
                }, this.opts.fpsInterval);
            }
        }
        else {
            this.ui.updateStatus("There is no ROM loaded, or it is invalid.");
        }
    },

    frame: function() {
        this.ppu.startFrame();
        var cycles = 0;
        var emulateSound = this.opts.emulateSound;
        var cpu = this.cpu;
        var ppu = this.ppu;
        var papu = this.papu;
        FRAMELOOP: for (;;) {
            if (cpu.cyclesToHalt === 0) {
                // Execute a CPU instruction
                cycles = cpu.emulate();
                if (emulateSound) {
                    papu.clockFrameCounter(cycles);
                }
                cycles *= 3;
            }
            else {
                if (cpu.cyclesToHalt > 8) {
                    cycles = 24;
                    if (emulateSound) {
                        papu.clockFrameCounter(8);
                    }
                    cpu.cyclesToHalt -= 8;
                }
                else {
                    cycles = cpu.cyclesToHalt * 3;
                    if (emulateSound) {
                        papu.clockFrameCounter(cpu.cyclesToHalt);
                    }
                    cpu.cyclesToHalt = 0;
                }
            }

            for (; cycles > 0; cycles--) {
                if(ppu.curX === ppu.spr0HitX &&
                        ppu.f_spVisibility === 1 &&
                        ppu.scanline - 21 === ppu.spr0HitY) {
                    // Set sprite 0 hit flag:
                    ppu.setStatusFlag(ppu.STATUS_SPRITE0HIT, true);
                }

                if (ppu.requestEndFrame) {
                    ppu.nmiCounter--;
                    if (ppu.nmiCounter === 0) {
                        ppu.requestEndFrame = false;
                        ppu.startVBlank();
                        break FRAMELOOP;
                    }
                }

                ppu.curX++;
                if (ppu.curX === 341) {
                    ppu.curX = 0;
                    ppu.endScanline();
                }
            }
        }
        this.fpsFrameCount++;
    },

    printFps: function() {
        var now = +new Date();
        var s = 'Running';
        if (this.lastFpsTime) {
            s += ': '+(
                this.fpsFrameCount / ((now - this.lastFpsTime) / 1000)
            ).toFixed(2)+' FPS';
        }
        this.ui.updateStatus(s);
        this.fpsFrameCount = 0;
        this.lastFpsTime = now;
    },

    stop: function() {
        clearInterval(this.frameInterval);
        clearInterval(this.fpsInterval);
        this.isRunning = false;
    },

    reloadRom: function() {
        if (this.romData !== null) {
            this.loadRom(this.romData);
        }
    },

    // Loads a ROM file into the CPU and PPU.
    // The ROM file is validated first.
    loadRom: function(data) {
        if (this.isRunning) {
            this.stop();
        }

        this.ui.updateStatus("Loading ROM...");

        // Load ROM file:
        this.rom = new JSNES.ROM(this);
        this.rom.load(data);

        if (this.rom.valid) {
            this.reset();
            this.mmap = this.rom.createMapper();
            if (!this.mmap) {
                return;
            }
            this.mmap.loadROM();
            this.ppu.setMirroring(this.rom.getMirroringType());
            this.romData = data;

            this.ui.updateStatus("Successfully loaded. Ready to be started.");
        }
        else {
            this.ui.updateStatus("Invalid ROM!");
        }
        return this.rom.valid;
    },

    resetFps: function() {
        this.lastFpsTime = null;
        this.fpsFrameCount = 0;
    },

    setFramerate: function(rate){
        this.opts.preferredFrameRate = rate;
        this.frameTime = 1000 / rate;
        this.papu.setSampleRate(this.opts.sampleRate, false);
    },

    toJSON: function() {
        return {
            'romData': this.romData,
            'cpu': this.cpu.toJSON(),
            'mmap': this.mmap.toJSON(),
            'ppu': this.ppu.toJSON()
        };
    },

    fromJSON: function(s) {
        this.loadRom(s.romData);
        this.cpu.fromJSON(s.cpu);
        this.mmap.fromJSON(s.mmap);
        this.ppu.fromJSON(s.ppu);
    }
};
/*
JSNES, based on Jamie Sanders' vNES
Copyright (C) 2010 Ben Firshman

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

JSNES.Utils = {
    copyArrayElements: function(src, srcPos, dest, destPos, length) {
        for (var i = 0; i < length; ++i) {
            dest[destPos + i] = src[srcPos + i];
        }
    },

    copyArray: function(src) {
        var dest = new Array(src.length);
        for (var i = 0; i < src.length; i++) {
            dest[i] = src[i];
        }
        return dest;
    },

    fromJSON: function(obj, state) {
        for (var i = 0; i < obj.JSON_PROPERTIES.length; i++) {
            obj[obj.JSON_PROPERTIES[i]] = state[obj.JSON_PROPERTIES[i]];
        }
    },

    toJSON: function(obj) {
        var state = {};
        for (var i = 0; i < obj.JSON_PROPERTIES.length; i++) {
            state[obj.JSON_PROPERTIES[i]] = obj[obj.JSON_PROPERTIES[i]];
        }
        return state;
    },

    isIE: function() {
        return (/msie/i.test(navigator.userAgent) && !/opera/i.test(navigator.userAgent));
    }
};

/*
JSNES, based on Jamie Sanders' vNES
Copyright (C) 2010 Ben Firshman

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

JSNES.CPU = function(nes) {
    this.nes = nes;

    // Keep Chrome happy
    this.mem = null;
    this.REG_ACC = null;
    this.REG_X = null;
    this.REG_Y = null;
    this.REG_SP = null;
    this.REG_PC = null;
    this.REG_PC_NEW = null;
    this.REG_STATUS = null;
    this.F_CARRY = null;
    this.F_DECIMAL = null;
    this.F_INTERRUPT = null;
    this.F_INTERRUPT_NEW = null;
    this.F_OVERFLOW = null;
    this.F_SIGN = null;
    this.F_ZERO = null;
    this.F_NOTUSED = null;
    this.F_NOTUSED_NEW = null;
    this.F_BRK = null;
    this.F_BRK_NEW = null;
    this.opdata = null;
    this.cyclesToHalt = null;
    this.crash = null;
    this.irqRequested = null;
    this.irqType = null;

    this.reset();
};

JSNES.CPU.prototype = {
    // IRQ Types
    IRQ_NORMAL: 0,
    IRQ_NMI: 1,
    IRQ_RESET: 2,

    reset: function() {
        // Main memory
        this.mem = new Array(0x10000);

        for (var i=0; i < 0x2000; i++) {
            this.mem[i] = 0xFF;
        }
        for (var p=0; p < 4; p++) {
            var i = p*0x800;
            this.mem[i+0x008] = 0xF7;
            this.mem[i+0x009] = 0xEF;
            this.mem[i+0x00A] = 0xDF;
            this.mem[i+0x00F] = 0xBF;
        }
        for (var i=0x2001; i < this.mem.length; i++) {
            this.mem[i] = 0;
        }

        // CPU Registers:
        this.REG_ACC = 0;
        this.REG_X = 0;
        this.REG_Y = 0;
        // Reset Stack pointer:
        this.REG_SP = 0x01FF;
        // Reset Program counter:
        this.REG_PC = 0x8000-1;
        this.REG_PC_NEW = 0x8000-1;
        // Reset Status register:
        this.REG_STATUS = 0x28;

        this.setStatus(0x28);

        // Set flags:
        this.F_CARRY = 0;
        this.F_DECIMAL = 0;
        this.F_INTERRUPT = 1;
        this.F_INTERRUPT_NEW = 1;
        this.F_OVERFLOW = 0;
        this.F_SIGN = 0;
        this.F_ZERO = 1;

        this.F_NOTUSED = 1;
        this.F_NOTUSED_NEW = 1;
        this.F_BRK = 1;
        this.F_BRK_NEW = 1;

        this.opdata = new JSNES.CPU.OpData().opdata;
        this.cyclesToHalt = 0;

        // Reset crash flag:
        this.crash = false;

        // Interrupt notification:
        this.irqRequested = false;
        this.irqType = null;

    },

    // Emulates a single CPU instruction, returns the number of cycles
    emulate: function() {
        var temp;
        var add;

        // Check interrupts:
        if(this.irqRequested){
            temp =
                (this.F_CARRY)|
                ((this.F_ZERO===0?1:0)<<1)|
                (this.F_INTERRUPT<<2)|
                (this.F_DECIMAL<<3)|
                (this.F_BRK<<4)|
                (this.F_NOTUSED<<5)|
                (this.F_OVERFLOW<<6)|
                (this.F_SIGN<<7);

            this.REG_PC_NEW = this.REG_PC;
            this.F_INTERRUPT_NEW = this.F_INTERRUPT;
            switch(this.irqType){
                case 0: {
                    // Normal IRQ:
                    if(this.F_INTERRUPT!=0){
                        // console.log("Interrupt was masked.");
                        break;
                    }
                    this.doIrq(temp);
                    // console.log("Did normal IRQ. I="+this.F_INTERRUPT);
                    break;
                }case 1:{
                    // NMI:
                    this.doNonMaskableInterrupt(temp);
                    break;

                }case 2:{
                    // Reset:
                    this.doResetInterrupt();
                    break;
                }
            }

            this.REG_PC = this.REG_PC_NEW;
            this.F_INTERRUPT = this.F_INTERRUPT_NEW;
            this.F_BRK = this.F_BRK_NEW;
            this.irqRequested = false;
        }

        var opinf = this.opdata[this.nes.mmap.load(this.REG_PC+1)];
        var cycleCount = (opinf>>24);
        var cycleAdd = 0;

        // Find address mode:
        var addrMode = (opinf >> 8) & 0xFF;

        // Increment PC by number of op bytes:
        var opaddr = this.REG_PC;
        this.REG_PC += ((opinf >> 16) & 0xFF);

        var addr = 0;
        switch(addrMode){
            case 0:{
                // Zero Page mode. Use the address given after the opcode,
                // but without high byte.
                addr = this.load(opaddr+2);
                break;

            }case 1:{
                // Relative mode.
                addr = this.load(opaddr+2);
                if(addr<0x80){
                    addr += this.REG_PC;
                }else{
                    addr += this.REG_PC-256;
                }
                break;
            }case 2:{
                // Ignore. Address is implied in instruction.
                break;
            }case 3:{
                // Absolute mode. Use the two bytes following the opcode as
                // an address.
                addr = this.load16bit(opaddr+2);
                break;
            }case 4:{
                // Accumulator mode. The address is in the accumulator
                // register.
                addr = this.REG_ACC;
                break;
            }case 5:{
                // Immediate mode. The value is given after the opcode.
                addr = this.REG_PC;
                break;
            }case 6:{
                // Zero Page Indexed mode, X as index. Use the address given
                // after the opcode, then add the
                // X register to it to get the final address.
                addr = (this.load(opaddr+2)+this.REG_X)&0xFF;
                break;
            }case 7:{
                // Zero Page Indexed mode, Y as index. Use the address given
                // after the opcode, then add the
                // Y register to it to get the final address.
                addr = (this.load(opaddr+2)+this.REG_Y)&0xFF;
                break;
            }case 8:{
                // Absolute Indexed Mode, X as index. Same as zero page
                // indexed, but with the high byte.
                addr = this.load16bit(opaddr+2);
                if((addr&0xFF00)!=((addr+this.REG_X)&0xFF00)){
                    cycleAdd = 1;
                }
                addr+=this.REG_X;
                break;
            }case 9:{
                // Absolute Indexed Mode, Y as index. Same as zero page
                // indexed, but with the high byte.
                addr = this.load16bit(opaddr+2);
                if((addr&0xFF00)!=((addr+this.REG_Y)&0xFF00)){
                    cycleAdd = 1;
                }
                addr+=this.REG_Y;
                break;
            }case 10:{
                // Pre-indexed Indirect mode. Find the 16-bit address
                // starting at the given location plus
                // the current X register. The value is the contents of that
                // address.
                addr = this.load(opaddr+2);
                if((addr&0xFF00)!=((addr+this.REG_X)&0xFF00)){
                    cycleAdd = 1;
                }
                addr+=this.REG_X;
                addr&=0xFF;
                addr = this.load16bit(addr);
                break;
            }case 11:{
                // Post-indexed Indirect mode. Find the 16-bit address
                // contained in the given location
                // (and the one following). Add to that address the contents
                // of the Y register. Fetch the value
                // stored at that adress.
                addr = this.load16bit(this.load(opaddr+2));
                if((addr&0xFF00)!=((addr+this.REG_Y)&0xFF00)){
                    cycleAdd = 1;
                }
                addr+=this.REG_Y;
                break;
            }case 12:{
                // Indirect Absolute mode. Find the 16-bit address contained
                // at the given location.
                addr = this.load16bit(opaddr+2);// Find op
                if(addr < 0x1FFF) {
                    addr = this.mem[addr] + (this.mem[(addr & 0xFF00) | (((addr & 0xFF) + 1) & 0xFF)] << 8);// Read from address given in op
                }
                else{
                    addr = this.nes.mmap.load(addr) + (this.nes.mmap.load((addr & 0xFF00) | (((addr & 0xFF) + 1) & 0xFF)) << 8);
                }
                break;

            }

        }
        // Wrap around for addresses above 0xFFFF:
        addr&=0xFFFF;

        // ----------------------------------------------------------------------------------------------------
        // Decode & execute instruction:
        // ----------------------------------------------------------------------------------------------------

        // This should be compiled to a jump table.
        switch(opinf&0xFF){
            case 0:{
                // *******
                // * ADC *
                // *******

                // Add with carry.
                temp = this.REG_ACC + this.load(addr) + this.F_CARRY;
                this.F_OVERFLOW = ((!(((this.REG_ACC ^ this.load(addr)) & 0x80)!=0) && (((this.REG_ACC ^ temp) & 0x80))!=0)?1:0);
                this.F_CARRY = (temp>255?1:0);
                this.F_SIGN = (temp>>7)&1;
                this.F_ZERO = temp&0xFF;
                this.REG_ACC = (temp&255);
                cycleCount+=cycleAdd;
                break;

            }case 1:{
                // *******
                // * AND *
                // *******

                // AND memory with accumulator.
                this.REG_ACC = this.REG_ACC & this.load(addr);
                this.F_SIGN = (this.REG_ACC>>7)&1;
                this.F_ZERO = this.REG_ACC;
                //this.REG_ACC = temp;
                if(addrMode!=11)cycleCount+=cycleAdd; // PostIdxInd = 11
                break;
            }case 2:{
                // *******
                // * ASL *
                // *******

                // Shift left one bit
                if(addrMode == 4){ // ADDR_ACC = 4

                    this.F_CARRY = (this.REG_ACC>>7)&1;
                    this.REG_ACC = (this.REG_ACC<<1)&255;
                    this.F_SIGN = (this.REG_ACC>>7)&1;
                    this.F_ZERO = this.REG_ACC;

                }else{

                    temp = this.load(addr);
                    this.F_CARRY = (temp>>7)&1;
                    temp = (temp<<1)&255;
                    this.F_SIGN = (temp>>7)&1;
                    this.F_ZERO = temp;
                    this.write(addr, temp);

                }
                break;

            }case 3:{

                // *******
                // * BCC *
                // *******

                // Branch on carry clear
                if(this.F_CARRY == 0){
                    cycleCount += ((opaddr&0xFF00)!=(addr&0xFF00)?2:1);
                    this.REG_PC = addr;
                }
                break;

            }case 4:{

                // *******
                // * BCS *
                // *******

                // Branch on carry set
                if(this.F_CARRY == 1){
                    cycleCount += ((opaddr&0xFF00)!=(addr&0xFF00)?2:1);
                    this.REG_PC = addr;
                }
                break;

            }case 5:{

                // *******
                // * BEQ *
                // *******

                // Branch on zero
                if(this.F_ZERO == 0){
                    cycleCount += ((opaddr&0xFF00)!=(addr&0xFF00)?2:1);
                    this.REG_PC = addr;
                }
                break;

            }case 6:{

                // *******
                // * BIT *
                // *******

                temp = this.load(addr);
                this.F_SIGN = (temp>>7)&1;
                this.F_OVERFLOW = (temp>>6)&1;
                temp &= this.REG_ACC;
                this.F_ZERO = temp;
                break;

            }case 7:{

                // *******
                // * BMI *
                // *******

                // Branch on negative result
                if(this.F_SIGN == 1){
                    cycleCount++;
                    this.REG_PC = addr;
                }
                break;

            }case 8:{

                // *******
                // * BNE *
                // *******

                // Branch on not zero
                if(this.F_ZERO != 0){
                    cycleCount += ((opaddr&0xFF00)!=(addr&0xFF00)?2:1);
                    this.REG_PC = addr;
                }
                break;

            }case 9:{

                // *******
                // * BPL *
                // *******

                // Branch on positive result
                if(this.F_SIGN == 0){
                    cycleCount += ((opaddr&0xFF00)!=(addr&0xFF00)?2:1);
                    this.REG_PC = addr;
                }
                break;

            }case 10:{

                // *******
                // * BRK *
                // *******

                this.REG_PC+=2;
                this.push((this.REG_PC>>8)&255);
                this.push(this.REG_PC&255);
                this.F_BRK = 1;

                this.push(
                    (this.F_CARRY)|
                    ((this.F_ZERO==0?1:0)<<1)|
                    (this.F_INTERRUPT<<2)|
                    (this.F_DECIMAL<<3)|
                    (this.F_BRK<<4)|
                    (this.F_NOTUSED<<5)|
                    (this.F_OVERFLOW<<6)|
                    (this.F_SIGN<<7)
                );

                this.F_INTERRUPT = 1;
                //this.REG_PC = load(0xFFFE) | (load(0xFFFF) << 8);
                this.REG_PC = this.load16bit(0xFFFE);
                this.REG_PC--;
                break;

            }case 11:{

                // *******
                // * BVC *
                // *******

                // Branch on overflow clear
                if(this.F_OVERFLOW == 0){
                    cycleCount += ((opaddr&0xFF00)!=(addr&0xFF00)?2:1);
                    this.REG_PC = addr;
                }
                break;

            }case 12:{

                // *******
                // * BVS *
                // *******

                // Branch on overflow set
                if(this.F_OVERFLOW == 1){
                    cycleCount += ((opaddr&0xFF00)!=(addr&0xFF00)?2:1);
                    this.REG_PC = addr;
                }
                break;

            }case 13:{

                // *******
                // * CLC *
                // *******

                // Clear carry flag
                this.F_CARRY = 0;
                break;

            }case 14:{

                // *******
                // * CLD *
                // *******

                // Clear decimal flag
                this.F_DECIMAL = 0;
                break;

            }case 15:{

                // *******
                // * CLI *
                // *******

                // Clear interrupt flag
                this.F_INTERRUPT = 0;
                break;

            }case 16:{

                // *******
                // * CLV *
                // *******

                // Clear overflow flag
                this.F_OVERFLOW = 0;
                break;

            }case 17:{

                // *******
                // * CMP *
                // *******

                // Compare memory and accumulator:
                temp = this.REG_ACC - this.load(addr);
                this.F_CARRY = (temp>=0?1:0);
                this.F_SIGN = (temp>>7)&1;
                this.F_ZERO = temp&0xFF;
                cycleCount+=cycleAdd;
                break;

            }case 18:{

                // *******
                // * CPX *
                // *******

                // Compare memory and index X:
                temp = this.REG_X - this.load(addr);
                this.F_CARRY = (temp>=0?1:0);
                this.F_SIGN = (temp>>7)&1;
                this.F_ZERO = temp&0xFF;
                break;

            }case 19:{

                // *******
                // * CPY *
                // *******

                // Compare memory and index Y:
                temp = this.REG_Y - this.load(addr);
                this.F_CARRY = (temp>=0?1:0);
                this.F_SIGN = (temp>>7)&1;
                this.F_ZERO = temp&0xFF;
                break;

            }case 20:{

                // *******
                // * DEC *
                // *******

                // Decrement memory by one:
                temp = (this.load(addr)-1)&0xFF;
                this.F_SIGN = (temp>>7)&1;
                this.F_ZERO = temp;
                this.write(addr, temp);
                break;

            }case 21:{

                // *******
                // * DEX *
                // *******

                // Decrement index X by one:
                this.REG_X = (this.REG_X-1)&0xFF;
                this.F_SIGN = (this.REG_X>>7)&1;
                this.F_ZERO = this.REG_X;
                break;

            }case 22:{

                // *******
                // * DEY *
                // *******

                // Decrement index Y by one:
                this.REG_Y = (this.REG_Y-1)&0xFF;
                this.F_SIGN = (this.REG_Y>>7)&1;
                this.F_ZERO = this.REG_Y;
                break;

            }case 23:{

                // *******
                // * EOR *
                // *******

                // XOR Memory with accumulator, store in accumulator:
                this.REG_ACC = (this.load(addr)^this.REG_ACC)&0xFF;
                this.F_SIGN = (this.REG_ACC>>7)&1;
                this.F_ZERO = this.REG_ACC;
                cycleCount+=cycleAdd;
                break;

            }case 24:{

                // *******
                // * INC *
                // *******

                // Increment memory by one:
                temp = (this.load(addr)+1)&0xFF;
                this.F_SIGN = (temp>>7)&1;
                this.F_ZERO = temp;
                this.write(addr, temp&0xFF);
                break;

            }case 25:{

                // *******
                // * INX *
                // *******

                // Increment index X by one:
                this.REG_X = (this.REG_X+1)&0xFF;
                this.F_SIGN = (this.REG_X>>7)&1;
                this.F_ZERO = this.REG_X;
                break;

            }case 26:{

                // *******
                // * INY *
                // *******

                // Increment index Y by one:
                this.REG_Y++;
                this.REG_Y &= 0xFF;
                this.F_SIGN = (this.REG_Y>>7)&1;
                this.F_ZERO = this.REG_Y;
                break;

            }case 27:{

                // *******
                // * JMP *
                // *******

                // Jump to new location:
                this.REG_PC = addr-1;
                break;

            }case 28:{

                // *******
                // * JSR *
                // *******

                // Jump to new location, saving return address.
                // Push return address on stack:
                this.push((this.REG_PC>>8)&255);
                this.push(this.REG_PC&255);
                this.REG_PC = addr-1;
                break;

            }case 29:{

                // *******
                // * LDA *
                // *******

                // Load accumulator with memory:
                this.REG_ACC = this.load(addr);
                this.F_SIGN = (this.REG_ACC>>7)&1;
                this.F_ZERO = this.REG_ACC;
                cycleCount+=cycleAdd;
                break;

            }case 30:{

                // *******
                // * LDX *
                // *******

                // Load index X with memory:
                this.REG_X = this.load(addr);
                this.F_SIGN = (this.REG_X>>7)&1;
                this.F_ZERO = this.REG_X;
                cycleCount+=cycleAdd;
                break;

            }case 31:{

                // *******
                // * LDY *
                // *******

                // Load index Y with memory:
                this.REG_Y = this.load(addr);
                this.F_SIGN = (this.REG_Y>>7)&1;
                this.F_ZERO = this.REG_Y;
                cycleCount+=cycleAdd;
                break;

            }case 32:{

                // *******
                // * LSR *
                // *******

                // Shift right one bit:
                if(addrMode == 4){ // ADDR_ACC

                    temp = (this.REG_ACC & 0xFF);
                    this.F_CARRY = temp&1;
                    temp >>= 1;
                    this.REG_ACC = temp;

                }else{

                    temp = this.load(addr) & 0xFF;
                    this.F_CARRY = temp&1;
                    temp >>= 1;
                    this.write(addr, temp);

                }
                this.F_SIGN = 0;
                this.F_ZERO = temp;
                break;

            }case 33:{

                // *******
                // * NOP *
                // *******

                // No OPeration.
                // Ignore.
                break;

            }case 34:{

                // *******
                // * ORA *
                // *******

                // OR memory with accumulator, store in accumulator.
                temp = (this.load(addr)|this.REG_ACC)&255;
                this.F_SIGN = (temp>>7)&1;
                this.F_ZERO = temp;
                this.REG_ACC = temp;
                if(addrMode!=11)cycleCount+=cycleAdd; // PostIdxInd = 11
                break;

            }case 35:{

                // *******
                // * PHA *
                // *******

                // Push accumulator on stack
                this.push(this.REG_ACC);
                break;

            }case 36:{

                // *******
                // * PHP *
                // *******

                // Push processor status on stack
                this.F_BRK = 1;
                this.push(
                    (this.F_CARRY)|
                    ((this.F_ZERO==0?1:0)<<1)|
                    (this.F_INTERRUPT<<2)|
                    (this.F_DECIMAL<<3)|
                    (this.F_BRK<<4)|
                    (this.F_NOTUSED<<5)|
                    (this.F_OVERFLOW<<6)|
                    (this.F_SIGN<<7)
                );
                break;

            }case 37:{

                // *******
                // * PLA *
                // *******

                // Pull accumulator from stack
                this.REG_ACC = this.pull();
                this.F_SIGN = (this.REG_ACC>>7)&1;
                this.F_ZERO = this.REG_ACC;
                break;

            }case 38:{

                // *******
                // * PLP *
                // *******

                // Pull processor status from stack
                temp = this.pull();
                this.F_CARRY     = (temp   )&1;
                this.F_ZERO      = (((temp>>1)&1)==1)?0:1;
                this.F_INTERRUPT = (temp>>2)&1;
                this.F_DECIMAL   = (temp>>3)&1;
                this.F_BRK       = (temp>>4)&1;
                this.F_NOTUSED   = (temp>>5)&1;
                this.F_OVERFLOW  = (temp>>6)&1;
                this.F_SIGN      = (temp>>7)&1;

                this.F_NOTUSED = 1;
                break;

            }case 39:{

                // *******
                // * ROL *
                // *******

                // Rotate one bit left
                if(addrMode == 4){ // ADDR_ACC = 4

                    temp = this.REG_ACC;
                    add = this.F_CARRY;
                    this.F_CARRY = (temp>>7)&1;
                    temp = ((temp<<1)&0xFF)+add;
                    this.REG_ACC = temp;

                }else{

                    temp = this.load(addr);
                    add = this.F_CARRY;
                    this.F_CARRY = (temp>>7)&1;
                    temp = ((temp<<1)&0xFF)+add;
                    this.write(addr, temp);

                }
                this.F_SIGN = (temp>>7)&1;
                this.F_ZERO = temp;
                break;

            }case 40:{

                // *******
                // * ROR *
                // *******

                // Rotate one bit right
                if(addrMode == 4){ // ADDR_ACC = 4

                    add = this.F_CARRY<<7;
                    this.F_CARRY = this.REG_ACC&1;
                    temp = (this.REG_ACC>>1)+add;
                    this.REG_ACC = temp;

                }else{

                    temp = this.load(addr);
                    add = this.F_CARRY<<7;
                    this.F_CARRY = temp&1;
                    temp = (temp>>1)+add;
                    this.write(addr, temp);

                }
                this.F_SIGN = (temp>>7)&1;
                this.F_ZERO = temp;
                break;

            }case 41:{

                // *******
                // * RTI *
                // *******

                // Return from interrupt. Pull status and PC from stack.

                temp = this.pull();
                this.F_CARRY     = (temp   )&1;
                this.F_ZERO      = ((temp>>1)&1)==0?1:0;
                this.F_INTERRUPT = (temp>>2)&1;
                this.F_DECIMAL   = (temp>>3)&1;
                this.F_BRK       = (temp>>4)&1;
                this.F_NOTUSED   = (temp>>5)&1;
                this.F_OVERFLOW  = (temp>>6)&1;
                this.F_SIGN      = (temp>>7)&1;

                this.REG_PC = this.pull();
                this.REG_PC += (this.pull()<<8);
                if(this.REG_PC==0xFFFF){
                    return;
                }
                this.REG_PC--;
                this.F_NOTUSED = 1;
                break;

            }case 42:{

                // *******
                // * RTS *
                // *******

                // Return from subroutine. Pull PC from stack.

                this.REG_PC = this.pull();
                this.REG_PC += (this.pull()<<8);

                if(this.REG_PC==0xFFFF){
                    return; // return from NSF play routine:
                }
                break;

            }case 43:{

                // *******
                // * SBC *
                // *******

                temp = this.REG_ACC-this.load(addr)-(1-this.F_CARRY);
                this.F_SIGN = (temp>>7)&1;
                this.F_ZERO = temp&0xFF;
                this.F_OVERFLOW = ((((this.REG_ACC^temp)&0x80)!=0 && ((this.REG_ACC^this.load(addr))&0x80)!=0)?1:0);
                this.F_CARRY = (temp<0?0:1);
                this.REG_ACC = (temp&0xFF);
                if(addrMode!=11)cycleCount+=cycleAdd; // PostIdxInd = 11
                break;

            }case 44:{

                // *******
                // * SEC *
                // *******

                // Set carry flag
                this.F_CARRY = 1;
                break;

            }case 45:{

                // *******
                // * SED *
                // *******

                // Set decimal mode
                this.F_DECIMAL = 1;
                break;

            }case 46:{

                // *******
                // * SEI *
                // *******

                // Set interrupt disable status
                this.F_INTERRUPT = 1;
                break;

            }case 47:{

                // *******
                // * STA *
                // *******

                // Store accumulator in memory
                this.write(addr, this.REG_ACC);
                break;

            }case 48:{

                // *******
                // * STX *
                // *******

                // Store index X in memory
                this.write(addr, this.REG_X);
                break;

            }case 49:{

                // *******
                // * STY *
                // *******

                // Store index Y in memory:
                this.write(addr, this.REG_Y);
                break;

            }case 50:{

                // *******
                // * TAX *
                // *******

                // Transfer accumulator to index X:
                this.REG_X = this.REG_ACC;
                this.F_SIGN = (this.REG_ACC>>7)&1;
                this.F_ZERO = this.REG_ACC;
                break;

            }case 51:{

                // *******
                // * TAY *
                // *******

                // Transfer accumulator to index Y:
                this.REG_Y = this.REG_ACC;
                this.F_SIGN = (this.REG_ACC>>7)&1;
                this.F_ZERO = this.REG_ACC;
                break;

            }case 52:{

                // *******
                // * TSX *
                // *******

                // Transfer stack pointer to index X:
                this.REG_X = (this.REG_SP-0x0100);
                this.F_SIGN = (this.REG_SP>>7)&1;
                this.F_ZERO = this.REG_X;
                break;

            }case 53:{

                // *******
                // * TXA *
                // *******

                // Transfer index X to accumulator:
                this.REG_ACC = this.REG_X;
                this.F_SIGN = (this.REG_X>>7)&1;
                this.F_ZERO = this.REG_X;
                break;

            }case 54:{

                // *******
                // * TXS *
                // *******

                // Transfer index X to stack pointer:
                this.REG_SP = (this.REG_X+0x0100);
                this.stackWrap();
                break;

            }case 55:{

                // *******
                // * TYA *
                // *******

                // Transfer index Y to accumulator:
                this.REG_ACC = this.REG_Y;
                this.F_SIGN = (this.REG_Y>>7)&1;
                this.F_ZERO = this.REG_Y;
                break;

            }default:{

                // *******
                // * ??? *
                // *******

                this.nes.stop();
                this.nes.crashMessage = "Game crashed, invalid opcode at address $"+opaddr.toString(16);
                break;

            }

        }// end of switch

        return cycleCount;

    },

    load: function(addr){
        if (addr < 0x2000) {
            return this.mem[addr & 0x7FF];
        }
        else {
            return this.nes.mmap.load(addr);
        }
    },

    load16bit: function(addr){
        if (addr < 0x1FFF) {
            return this.mem[addr&0x7FF]
                | (this.mem[(addr+1)&0x7FF]<<8);
        }
        else {
            return this.nes.mmap.load(addr) | (this.nes.mmap.load(addr+1) << 8);
        }
    },

    write: function(addr, val){
        if(addr < 0x2000) {
            this.mem[addr&0x7FF] = val;
        }
        else {
            this.nes.mmap.write(addr,val);
        }
    },

    requestIrq: function(type){
        if(this.irqRequested){
            if(type == this.IRQ_NORMAL){
                return;
            }
            // console.log("too fast irqs. type="+type);
        }
        this.irqRequested = true;
        this.irqType = type;
    },

    push: function(value){
        this.nes.mmap.write(this.REG_SP, value);
        this.REG_SP--;
        this.REG_SP = 0x0100 | (this.REG_SP&0xFF);
    },

    stackWrap: function(){
        this.REG_SP = 0x0100 | (this.REG_SP&0xFF);
    },

    pull: function(){
        this.REG_SP++;
        this.REG_SP = 0x0100 | (this.REG_SP&0xFF);
        return this.nes.mmap.load(this.REG_SP);
    },

    pageCrossed: function(addr1, addr2){
        return ((addr1&0xFF00) != (addr2&0xFF00));
    },

    haltCycles: function(cycles){
        this.cyclesToHalt += cycles;
    },

    doNonMaskableInterrupt: function(status){
        if((this.nes.mmap.load(0x2000) & 128) != 0) { // Check whether VBlank Interrupts are enabled

            this.REG_PC_NEW++;
            this.push((this.REG_PC_NEW>>8)&0xFF);
            this.push(this.REG_PC_NEW&0xFF);
            //this.F_INTERRUPT_NEW = 1;
            this.push(status);

            this.REG_PC_NEW = this.nes.mmap.load(0xFFFA) | (this.nes.mmap.load(0xFFFB) << 8);
            this.REG_PC_NEW--;
        }
    },

    doResetInterrupt: function(){
        this.REG_PC_NEW = this.nes.mmap.load(0xFFFC) | (this.nes.mmap.load(0xFFFD) << 8);
        this.REG_PC_NEW--;
    },

    doIrq: function(status){
        this.REG_PC_NEW++;
        this.push((this.REG_PC_NEW>>8)&0xFF);
        this.push(this.REG_PC_NEW&0xFF);
        this.push(status);
        this.F_INTERRUPT_NEW = 1;
        this.F_BRK_NEW = 0;

        this.REG_PC_NEW = this.nes.mmap.load(0xFFFE) | (this.nes.mmap.load(0xFFFF) << 8);
        this.REG_PC_NEW--;
    },

    getStatus: function(){
        return (this.F_CARRY)
                |(this.F_ZERO<<1)
                |(this.F_INTERRUPT<<2)
                |(this.F_DECIMAL<<3)
                |(this.F_BRK<<4)
                |(this.F_NOTUSED<<5)
                |(this.F_OVERFLOW<<6)
                |(this.F_SIGN<<7);
    },

    setStatus: function(st){
        this.F_CARRY     = (st   )&1;
        this.F_ZERO      = (st>>1)&1;
        this.F_INTERRUPT = (st>>2)&1;
        this.F_DECIMAL   = (st>>3)&1;
        this.F_BRK       = (st>>4)&1;
        this.F_NOTUSED   = (st>>5)&1;
        this.F_OVERFLOW  = (st>>6)&1;
        this.F_SIGN      = (st>>7)&1;
    },

    JSON_PROPERTIES: [
        'mem', 'cyclesToHalt', 'irqRequested', 'irqType',
        // Registers
        'REG_ACC', 'REG_X', 'REG_Y', 'REG_SP', 'REG_PC', 'REG_PC_NEW',
        'REG_STATUS',
        // Status
        'F_CARRY', 'F_DECIMAL', 'F_INTERRUPT', 'F_INTERRUPT_NEW', 'F_OVERFLOW',
        'F_SIGN', 'F_ZERO', 'F_NOTUSED', 'F_NOTUSED_NEW', 'F_BRK', 'F_BRK_NEW'
    ],

    toJSON: function() {
        return JSNES.Utils.toJSON(this);
    },

    fromJSON: function(s) {
        JSNES.Utils.fromJSON(this, s);
    }
}

// Generates and provides an array of details about instructions
JSNES.CPU.OpData = function() {
    this.opdata = new Array(256);

    // Set all to invalid instruction (to detect crashes):
    for(var i=0;i<256;i++) this.opdata[i]=0xFF;

    // Now fill in all valid opcodes:

    // ADC:
    this.setOp(this.INS_ADC,0x69,this.ADDR_IMM,2,2);
    this.setOp(this.INS_ADC,0x65,this.ADDR_ZP,2,3);
    this.setOp(this.INS_ADC,0x75,this.ADDR_ZPX,2,4);
    this.setOp(this.INS_ADC,0x6D,this.ADDR_ABS,3,4);
    this.setOp(this.INS_ADC,0x7D,this.ADDR_ABSX,3,4);
    this.setOp(this.INS_ADC,0x79,this.ADDR_ABSY,3,4);
    this.setOp(this.INS_ADC,0x61,this.ADDR_PREIDXIND,2,6);
    this.setOp(this.INS_ADC,0x71,this.ADDR_POSTIDXIND,2,5);

    // AND:
    this.setOp(this.INS_AND,0x29,this.ADDR_IMM,2,2);
    this.setOp(this.INS_AND,0x25,this.ADDR_ZP,2,3);
    this.setOp(this.INS_AND,0x35,this.ADDR_ZPX,2,4);
    this.setOp(this.INS_AND,0x2D,this.ADDR_ABS,3,4);
    this.setOp(this.INS_AND,0x3D,this.ADDR_ABSX,3,4);
    this.setOp(this.INS_AND,0x39,this.ADDR_ABSY,3,4);
    this.setOp(this.INS_AND,0x21,this.ADDR_PREIDXIND,2,6);
    this.setOp(this.INS_AND,0x31,this.ADDR_POSTIDXIND,2,5);

    // ASL:
    this.setOp(this.INS_ASL,0x0A,this.ADDR_ACC,1,2);
    this.setOp(this.INS_ASL,0x06,this.ADDR_ZP,2,5);
    this.setOp(this.INS_ASL,0x16,this.ADDR_ZPX,2,6);
    this.setOp(this.INS_ASL,0x0E,this.ADDR_ABS,3,6);
    this.setOp(this.INS_ASL,0x1E,this.ADDR_ABSX,3,7);

    // BCC:
    this.setOp(this.INS_BCC,0x90,this.ADDR_REL,2,2);

    // BCS:
    this.setOp(this.INS_BCS,0xB0,this.ADDR_REL,2,2);

    // BEQ:
    this.setOp(this.INS_BEQ,0xF0,this.ADDR_REL,2,2);

    // BIT:
    this.setOp(this.INS_BIT,0x24,this.ADDR_ZP,2,3);
    this.setOp(this.INS_BIT,0x2C,this.ADDR_ABS,3,4);

    // BMI:
    this.setOp(this.INS_BMI,0x30,this.ADDR_REL,2,2);

    // BNE:
    this.setOp(this.INS_BNE,0xD0,this.ADDR_REL,2,2);

    // BPL:
    this.setOp(this.INS_BPL,0x10,this.ADDR_REL,2,2);

    // BRK:
    this.setOp(this.INS_BRK,0x00,this.ADDR_IMP,1,7);

    // BVC:
    this.setOp(this.INS_BVC,0x50,this.ADDR_REL,2,2);

    // BVS:
    this.setOp(this.INS_BVS,0x70,this.ADDR_REL,2,2);

    // CLC:
    this.setOp(this.INS_CLC,0x18,this.ADDR_IMP,1,2);

    // CLD:
    this.setOp(this.INS_CLD,0xD8,this.ADDR_IMP,1,2);

    // CLI:
    this.setOp(this.INS_CLI,0x58,this.ADDR_IMP,1,2);

    // CLV:
    this.setOp(this.INS_CLV,0xB8,this.ADDR_IMP,1,2);

    // CMP:
    this.setOp(this.INS_CMP,0xC9,this.ADDR_IMM,2,2);
    this.setOp(this.INS_CMP,0xC5,this.ADDR_ZP,2,3);
    this.setOp(this.INS_CMP,0xD5,this.ADDR_ZPX,2,4);
    this.setOp(this.INS_CMP,0xCD,this.ADDR_ABS,3,4);
    this.setOp(this.INS_CMP,0xDD,this.ADDR_ABSX,3,4);
    this.setOp(this.INS_CMP,0xD9,this.ADDR_ABSY,3,4);
    this.setOp(this.INS_CMP,0xC1,this.ADDR_PREIDXIND,2,6);
    this.setOp(this.INS_CMP,0xD1,this.ADDR_POSTIDXIND,2,5);

    // CPX:
    this.setOp(this.INS_CPX,0xE0,this.ADDR_IMM,2,2);
    this.setOp(this.INS_CPX,0xE4,this.ADDR_ZP,2,3);
    this.setOp(this.INS_CPX,0xEC,this.ADDR_ABS,3,4);

    // CPY:
    this.setOp(this.INS_CPY,0xC0,this.ADDR_IMM,2,2);
    this.setOp(this.INS_CPY,0xC4,this.ADDR_ZP,2,3);
    this.setOp(this.INS_CPY,0xCC,this.ADDR_ABS,3,4);

    // DEC:
    this.setOp(this.INS_DEC,0xC6,this.ADDR_ZP,2,5);
    this.setOp(this.INS_DEC,0xD6,this.ADDR_ZPX,2,6);
    this.setOp(this.INS_DEC,0xCE,this.ADDR_ABS,3,6);
    this.setOp(this.INS_DEC,0xDE,this.ADDR_ABSX,3,7);

    // DEX:
    this.setOp(this.INS_DEX,0xCA,this.ADDR_IMP,1,2);

    // DEY:
    this.setOp(this.INS_DEY,0x88,this.ADDR_IMP,1,2);

    // EOR:
    this.setOp(this.INS_EOR,0x49,this.ADDR_IMM,2,2);
    this.setOp(this.INS_EOR,0x45,this.ADDR_ZP,2,3);
    this.setOp(this.INS_EOR,0x55,this.ADDR_ZPX,2,4);
    this.setOp(this.INS_EOR,0x4D,this.ADDR_ABS,3,4);
    this.setOp(this.INS_EOR,0x5D,this.ADDR_ABSX,3,4);
    this.setOp(this.INS_EOR,0x59,this.ADDR_ABSY,3,4);
    this.setOp(this.INS_EOR,0x41,this.ADDR_PREIDXIND,2,6);
    this.setOp(this.INS_EOR,0x51,this.ADDR_POSTIDXIND,2,5);

    // INC:
    this.setOp(this.INS_INC,0xE6,this.ADDR_ZP,2,5);
    this.setOp(this.INS_INC,0xF6,this.ADDR_ZPX,2,6);
    this.setOp(this.INS_INC,0xEE,this.ADDR_ABS,3,6);
    this.setOp(this.INS_INC,0xFE,this.ADDR_ABSX,3,7);

    // INX:
    this.setOp(this.INS_INX,0xE8,this.ADDR_IMP,1,2);

    // INY:
    this.setOp(this.INS_INY,0xC8,this.ADDR_IMP,1,2);

    // JMP:
    this.setOp(this.INS_JMP,0x4C,this.ADDR_ABS,3,3);
    this.setOp(this.INS_JMP,0x6C,this.ADDR_INDABS,3,5);

    // JSR:
    this.setOp(this.INS_JSR,0x20,this.ADDR_ABS,3,6);

    // LDA:
    this.setOp(this.INS_LDA,0xA9,this.ADDR_IMM,2,2);
    this.setOp(this.INS_LDA,0xA5,this.ADDR_ZP,2,3);
    this.setOp(this.INS_LDA,0xB5,this.ADDR_ZPX,2,4);
    this.setOp(this.INS_LDA,0xAD,this.ADDR_ABS,3,4);
    this.setOp(this.INS_LDA,0xBD,this.ADDR_ABSX,3,4);
    this.setOp(this.INS_LDA,0xB9,this.ADDR_ABSY,3,4);
    this.setOp(this.INS_LDA,0xA1,this.ADDR_PREIDXIND,2,6);
    this.setOp(this.INS_LDA,0xB1,this.ADDR_POSTIDXIND,2,5);


    // LDX:
    this.setOp(this.INS_LDX,0xA2,this.ADDR_IMM,2,2);
    this.setOp(this.INS_LDX,0xA6,this.ADDR_ZP,2,3);
    this.setOp(this.INS_LDX,0xB6,this.ADDR_ZPY,2,4);
    this.setOp(this.INS_LDX,0xAE,this.ADDR_ABS,3,4);
    this.setOp(this.INS_LDX,0xBE,this.ADDR_ABSY,3,4);

    // LDY:
    this.setOp(this.INS_LDY,0xA0,this.ADDR_IMM,2,2);
    this.setOp(this.INS_LDY,0xA4,this.ADDR_ZP,2,3);
    this.setOp(this.INS_LDY,0xB4,this.ADDR_ZPX,2,4);
    this.setOp(this.INS_LDY,0xAC,this.ADDR_ABS,3,4);
    this.setOp(this.INS_LDY,0xBC,this.ADDR_ABSX,3,4);

    // LSR:
    this.setOp(this.INS_LSR,0x4A,this.ADDR_ACC,1,2);
    this.setOp(this.INS_LSR,0x46,this.ADDR_ZP,2,5);
    this.setOp(this.INS_LSR,0x56,this.ADDR_ZPX,2,6);
    this.setOp(this.INS_LSR,0x4E,this.ADDR_ABS,3,6);
    this.setOp(this.INS_LSR,0x5E,this.ADDR_ABSX,3,7);

    // NOP:
    this.setOp(this.INS_NOP,0xEA,this.ADDR_IMP,1,2);

    // ORA:
    this.setOp(this.INS_ORA,0x09,this.ADDR_IMM,2,2);
    this.setOp(this.INS_ORA,0x05,this.ADDR_ZP,2,3);
    this.setOp(this.INS_ORA,0x15,this.ADDR_ZPX,2,4);
    this.setOp(this.INS_ORA,0x0D,this.ADDR_ABS,3,4);
    this.setOp(this.INS_ORA,0x1D,this.ADDR_ABSX,3,4);
    this.setOp(this.INS_ORA,0x19,this.ADDR_ABSY,3,4);
    this.setOp(this.INS_ORA,0x01,this.ADDR_PREIDXIND,2,6);
    this.setOp(this.INS_ORA,0x11,this.ADDR_POSTIDXIND,2,5);

    // PHA:
    this.setOp(this.INS_PHA,0x48,this.ADDR_IMP,1,3);

    // PHP:
    this.setOp(this.INS_PHP,0x08,this.ADDR_IMP,1,3);

    // PLA:
    this.setOp(this.INS_PLA,0x68,this.ADDR_IMP,1,4);

    // PLP:
    this.setOp(this.INS_PLP,0x28,this.ADDR_IMP,1,4);

    // ROL:
    this.setOp(this.INS_ROL,0x2A,this.ADDR_ACC,1,2);
    this.setOp(this.INS_ROL,0x26,this.ADDR_ZP,2,5);
    this.setOp(this.INS_ROL,0x36,this.ADDR_ZPX,2,6);
    this.setOp(this.INS_ROL,0x2E,this.ADDR_ABS,3,6);
    this.setOp(this.INS_ROL,0x3E,this.ADDR_ABSX,3,7);

    // ROR:
    this.setOp(this.INS_ROR,0x6A,this.ADDR_ACC,1,2);
    this.setOp(this.INS_ROR,0x66,this.ADDR_ZP,2,5);
    this.setOp(this.INS_ROR,0x76,this.ADDR_ZPX,2,6);
    this.setOp(this.INS_ROR,0x6E,this.ADDR_ABS,3,6);
    this.setOp(this.INS_ROR,0x7E,this.ADDR_ABSX,3,7);

    // RTI:
    this.setOp(this.INS_RTI,0x40,this.ADDR_IMP,1,6);

    // RTS:
    this.setOp(this.INS_RTS,0x60,this.ADDR_IMP,1,6);

    // SBC:
    this.setOp(this.INS_SBC,0xE9,this.ADDR_IMM,2,2);
    this.setOp(this.INS_SBC,0xE5,this.ADDR_ZP,2,3);
    this.setOp(this.INS_SBC,0xF5,this.ADDR_ZPX,2,4);
    this.setOp(this.INS_SBC,0xED,this.ADDR_ABS,3,4);
    this.setOp(this.INS_SBC,0xFD,this.ADDR_ABSX,3,4);
    this.setOp(this.INS_SBC,0xF9,this.ADDR_ABSY,3,4);
    this.setOp(this.INS_SBC,0xE1,this.ADDR_PREIDXIND,2,6);
    this.setOp(this.INS_SBC,0xF1,this.ADDR_POSTIDXIND,2,5);

    // SEC:
    this.setOp(this.INS_SEC,0x38,this.ADDR_IMP,1,2);

    // SED:
    this.setOp(this.INS_SED,0xF8,this.ADDR_IMP,1,2);

    // SEI:
    this.setOp(this.INS_SEI,0x78,this.ADDR_IMP,1,2);

    // STA:
    this.setOp(this.INS_STA,0x85,this.ADDR_ZP,2,3);
    this.setOp(this.INS_STA,0x95,this.ADDR_ZPX,2,4);
    this.setOp(this.INS_STA,0x8D,this.ADDR_ABS,3,4);
    this.setOp(this.INS_STA,0x9D,this.ADDR_ABSX,3,5);
    this.setOp(this.INS_STA,0x99,this.ADDR_ABSY,3,5);
    this.setOp(this.INS_STA,0x81,this.ADDR_PREIDXIND,2,6);
    this.setOp(this.INS_STA,0x91,this.ADDR_POSTIDXIND,2,6);

    // STX:
    this.setOp(this.INS_STX,0x86,this.ADDR_ZP,2,3);
    this.setOp(this.INS_STX,0x96,this.ADDR_ZPY,2,4);
    this.setOp(this.INS_STX,0x8E,this.ADDR_ABS,3,4);

    // STY:
    this.setOp(this.INS_STY,0x84,this.ADDR_ZP,2,3);
    this.setOp(this.INS_STY,0x94,this.ADDR_ZPX,2,4);
    this.setOp(this.INS_STY,0x8C,this.ADDR_ABS,3,4);

    // TAX:
    this.setOp(this.INS_TAX,0xAA,this.ADDR_IMP,1,2);

    // TAY:
    this.setOp(this.INS_TAY,0xA8,this.ADDR_IMP,1,2);

    // TSX:
    this.setOp(this.INS_TSX,0xBA,this.ADDR_IMP,1,2);

    // TXA:
    this.setOp(this.INS_TXA,0x8A,this.ADDR_IMP,1,2);

    // TXS:
    this.setOp(this.INS_TXS,0x9A,this.ADDR_IMP,1,2);

    // TYA:
    this.setOp(this.INS_TYA,0x98,this.ADDR_IMP,1,2);

    this.cycTable = new Array(
    /*0x00*/ 7,6,2,8,3,3,5,5,3,2,2,2,4,4,6,6,
    /*0x10*/ 2,5,2,8,4,4,6,6,2,4,2,7,4,4,7,7,
    /*0x20*/ 6,6,2,8,3,3,5,5,4,2,2,2,4,4,6,6,
    /*0x30*/ 2,5,2,8,4,4,6,6,2,4,2,7,4,4,7,7,
    /*0x40*/ 6,6,2,8,3,3,5,5,3,2,2,2,3,4,6,6,
    /*0x50*/ 2,5,2,8,4,4,6,6,2,4,2,7,4,4,7,7,
    /*0x60*/ 6,6,2,8,3,3,5,5,4,2,2,2,5,4,6,6,
    /*0x70*/ 2,5,2,8,4,4,6,6,2,4,2,7,4,4,7,7,
    /*0x80*/ 2,6,2,6,3,3,3,3,2,2,2,2,4,4,4,4,
    /*0x90*/ 2,6,2,6,4,4,4,4,2,5,2,5,5,5,5,5,
    /*0xA0*/ 2,6,2,6,3,3,3,3,2,2,2,2,4,4,4,4,
    /*0xB0*/ 2,5,2,5,4,4,4,4,2,4,2,4,4,4,4,4,
    /*0xC0*/ 2,6,2,8,3,3,5,5,2,2,2,2,4,4,6,6,
    /*0xD0*/ 2,5,2,8,4,4,6,6,2,4,2,7,4,4,7,7,
    /*0xE0*/ 2,6,3,8,3,3,5,5,2,2,2,2,4,4,6,6,
    /*0xF0*/ 2,5,2,8,4,4,6,6,2,4,2,7,4,4,7,7
    );


    this.instname = new Array(56);

    // Instruction Names:
    this.instname[ 0] = "ADC";
    this.instname[ 1] = "AND";
    this.instname[ 2] = "ASL";
    this.instname[ 3] = "BCC";
    this.instname[ 4] = "BCS";
    this.instname[ 5] = "BEQ";
    this.instname[ 6] = "BIT";
    this.instname[ 7] = "BMI";
    this.instname[ 8] = "BNE";
    this.instname[ 9] = "BPL";
    this.instname[10] = "BRK";
    this.instname[11] = "BVC";
    this.instname[12] = "BVS";
    this.instname[13] = "CLC";
    this.instname[14] = "CLD";
    this.instname[15] = "CLI";
    this.instname[16] = "CLV";
    this.instname[17] = "CMP";
    this.instname[18] = "CPX";
    this.instname[19] = "CPY";
    this.instname[20] = "DEC";
    this.instname[21] = "DEX";
    this.instname[22] = "DEY";
    this.instname[23] = "EOR";
    this.instname[24] = "INC";
    this.instname[25] = "INX";
    this.instname[26] = "INY";
    this.instname[27] = "JMP";
    this.instname[28] = "JSR";
    this.instname[29] = "LDA";
    this.instname[30] = "LDX";
    this.instname[31] = "LDY";
    this.instname[32] = "LSR";
    this.instname[33] = "NOP";
    this.instname[34] = "ORA";
    this.instname[35] = "PHA";
    this.instname[36] = "PHP";
    this.instname[37] = "PLA";
    this.instname[38] = "PLP";
    this.instname[39] = "ROL";
    this.instname[40] = "ROR";
    this.instname[41] = "RTI";
    this.instname[42] = "RTS";
    this.instname[43] = "SBC";
    this.instname[44] = "SEC";
    this.instname[45] = "SED";
    this.instname[46] = "SEI";
    this.instname[47] = "STA";
    this.instname[48] = "STX";
    this.instname[49] = "STY";
    this.instname[50] = "TAX";
    this.instname[51] = "TAY";
    this.instname[52] = "TSX";
    this.instname[53] = "TXA";
    this.instname[54] = "TXS";
    this.instname[55] = "TYA";

    this.addrDesc = new Array(
        "Zero Page           ",
        "Relative            ",
        "Implied             ",
        "Absolute            ",
        "Accumulator         ",
        "Immediate           ",
        "Zero Page,X         ",
        "Zero Page,Y         ",
        "Absolute,X          ",
        "Absolute,Y          ",
        "Preindexed Indirect ",
        "Postindexed Indirect",
        "Indirect Absolute   "
    );
}

JSNES.CPU.OpData.prototype = {
    INS_ADC: 0,
    INS_AND: 1,
    INS_ASL: 2,

    INS_BCC: 3,
    INS_BCS: 4,
    INS_BEQ: 5,
    INS_BIT: 6,
    INS_BMI: 7,
    INS_BNE: 8,
    INS_BPL: 9,
    INS_BRK: 10,
    INS_BVC: 11,
    INS_BVS: 12,

    INS_CLC: 13,
    INS_CLD: 14,
    INS_CLI: 15,
    INS_CLV: 16,
    INS_CMP: 17,
    INS_CPX: 18,
    INS_CPY: 19,

    INS_DEC: 20,
    INS_DEX: 21,
    INS_DEY: 22,

    INS_EOR: 23,

    INS_INC: 24,
    INS_INX: 25,
    INS_INY: 26,

    INS_JMP: 27,
    INS_JSR: 28,

    INS_LDA: 29,
    INS_LDX: 30,
    INS_LDY: 31,
    INS_LSR: 32,

    INS_NOP: 33,

    INS_ORA: 34,

    INS_PHA: 35,
    INS_PHP: 36,
    INS_PLA: 37,
    INS_PLP: 38,

    INS_ROL: 39,
    INS_ROR: 40,
    INS_RTI: 41,
    INS_RTS: 42,

    INS_SBC: 43,
    INS_SEC: 44,
    INS_SED: 45,
    INS_SEI: 46,
    INS_STA: 47,
    INS_STX: 48,
    INS_STY: 49,

    INS_TAX: 50,
    INS_TAY: 51,
    INS_TSX: 52,
    INS_TXA: 53,
    INS_TXS: 54,
    INS_TYA: 55,

    INS_DUMMY: 56, // dummy instruction used for 'halting' the processor some cycles

    // -------------------------------- //

    // Addressing modes:
    ADDR_ZP        : 0,
    ADDR_REL       : 1,
    ADDR_IMP       : 2,
    ADDR_ABS       : 3,
    ADDR_ACC       : 4,
    ADDR_IMM       : 5,
    ADDR_ZPX       : 6,
    ADDR_ZPY       : 7,
    ADDR_ABSX      : 8,
    ADDR_ABSY      : 9,
    ADDR_PREIDXIND : 10,
    ADDR_POSTIDXIND: 11,
    ADDR_INDABS    : 12,

    setOp: function(inst, op, addr, size, cycles){
        this.opdata[op] =
            ((inst  &0xFF)    )|
            ((addr  &0xFF)<< 8)|
            ((size  &0xFF)<<16)|
            ((cycles&0xFF)<<24);
    }
};
/*
JSNES, based on Jamie Sanders' vNES
Copyright (C) 2010 Ben Firshman

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

// Keyboard events are bound in the UI
JSNES.Keyboard = function() {
    var i;

    this.keys = {
        KEY_A: 0,
        KEY_B: 1,
        KEY_SELECT: 2,
        KEY_START: 3,
        KEY_UP: 4,
        KEY_DOWN: 5,
        KEY_LEFT: 6,
        KEY_RIGHT: 7
    };

    this.state1 = new Array(8);
    for (i = 0; i < this.state1.length; i++) {
        this.state1[i] = 0x40;
    }
    this.state2 = new Array(8);
    for (i = 0; i < this.state2.length; i++) {
        this.state2[i] = 0x40;
    }
};

JSNES.Keyboard.prototype = {
    setKey: function(key, value) {
        switch (key) {
            case 88: this.state1[this.keys.KEY_A] = value; break;      // X
            case 89: this.state1[this.keys.KEY_B] = value; break;      // Y (Central European keyboard)
            case 90: this.state1[this.keys.KEY_B] = value; break;      // Z
            case 17: this.state1[this.keys.KEY_SELECT] = value; break; // Right Ctrl
            case 13: this.state1[this.keys.KEY_START] = value; break;  // Enter
            case 38: this.state1[this.keys.KEY_UP] = value; break;     // Up
            case 40: this.state1[this.keys.KEY_DOWN] = value; break;   // Down
            case 37: this.state1[this.keys.KEY_LEFT] = value; break;   // Left
            case 39: this.state1[this.keys.KEY_RIGHT] = value; break;  // Right

            case 103: this.state2[this.keys.KEY_A] = value; break;     // Num-7
            case 105: this.state2[this.keys.KEY_B] = value; break;     // Num-9
            case 99: this.state2[this.keys.KEY_SELECT] = value; break; // Num-3
            case 97: this.state2[this.keys.KEY_START] = value; break;  // Num-1
            case 104: this.state2[this.keys.KEY_UP] = value; break;    // Num-8
            case 98: this.state2[this.keys.KEY_DOWN] = value; break;   // Num-2
            case 100: this.state2[this.keys.KEY_LEFT] = value; break;  // Num-4
            case 102: this.state2[this.keys.KEY_RIGHT] = value; break; // Num-6
            default: return true;
        }
        return false; // preventDefault
    },

    keyDown: function(evt) {
        if (!this.setKey(evt.keyCode, 0x41) && evt.preventDefault) {
            evt.preventDefault();
        }
    },

    keyUp: function(evt) {
        if (!this.setKey(evt.keyCode, 0x40) && evt.preventDefault) {
            evt.preventDefault();
        }
    },

    keyPress: function(evt) {
        evt.preventDefault();
    }
};
/*
JSNES, based on Jamie Sanders' vNES
Copyright (C) 2010 Ben Firshman

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

JSNES.Mappers = {};

JSNES.Mappers[0] = function(nes) {
    this.nes = nes;
};

JSNES.Mappers[0].prototype = {
    reset: function() {
        this.joy1StrobeState = 0;
        this.joy2StrobeState = 0;
        this.joypadLastWrite = 0;

        this.mousePressed = false;
        this.mouseX = null;
        this.mouseY = null;
    },

    write: function(address, value) {
        if (address < 0x2000) {
            // Mirroring of RAM:
            this.nes.cpu.mem[address & 0x7FF] = value;

        }
        else if (address > 0x4017) {
            this.nes.cpu.mem[address] = value;
            if (address >= 0x6000 && address < 0x8000) {
                // Write to SaveRAM. Store in file:
                // TODO: not yet
                //if(this.nes.rom!=null)
                //    this.nes.rom.writeBatteryRam(address,value);
            }
        }
        else if (address > 0x2007 && address < 0x4000) {
            this.regWrite(0x2000 + (address & 0x7), value);
        }
        else {
            this.regWrite(address, value);
        }
    },

    writelow: function(address, value) {
        if (address < 0x2000) {
            // Mirroring of RAM:
            this.nes.cpu.mem[address & 0x7FF] = value;
        }
        else if (address > 0x4017) {
            this.nes.cpu.mem[address] = value;
        }
        else if (address > 0x2007 && address < 0x4000) {
            this.regWrite(0x2000 + (address & 0x7), value);
        }
        else {
            this.regWrite(address, value);
        }
    },

    load: function(address) {
        // Wrap around:
        address &= 0xFFFF;

        // Check address range:
        if (address > 0x4017) {
            // ROM:
            return this.nes.cpu.mem[address];
        }
        else if (address >= 0x2000) {
            // I/O Ports.
            return this.regLoad(address);
        }
        else {
            // RAM (mirrored)
            return this.nes.cpu.mem[address & 0x7FF];
        }
    },

    regLoad: function(address) {
        switch (address >> 12) { // use fourth nibble (0xF000)
            case 0:
                break;

            case 1:
                break;

            case 2:
                // Fall through to case 3
            case 3:
                // PPU Registers
                switch (address & 0x7) {
                    case 0x0:
                        // 0x2000:
                        // PPU Control Register 1.
                        // (the value is stored both
                        // in main memory and in the
                        // PPU as flags):
                        // (not in the real NES)
                        return this.nes.cpu.mem[0x2000];

                    case 0x1:
                        // 0x2001:
                        // PPU Control Register 2.
                        // (the value is stored both
                        // in main memory and in the
                        // PPU as flags):
                        // (not in the real NES)
                        return this.nes.cpu.mem[0x2001];

                    case 0x2:
                        // 0x2002:
                        // PPU Status Register.
                        // The value is stored in
                        // main memory in addition
                        // to as flags in the PPU.
                        // (not in the real NES)
                        return this.nes.ppu.readStatusRegister();

                    case 0x3:
                        return 0;

                    case 0x4:
                        // 0x2004:
                        // Sprite Memory read.
                        return this.nes.ppu.sramLoad();
                    case 0x5:
                        return 0;

                    case 0x6:
                        return 0;

                    case 0x7:
                        // 0x2007:
                        // VRAM read:
                        return this.nes.ppu.vramLoad();
                }
                break;
            case 4:
                // Sound+Joypad registers
                switch (address - 0x4015) {
                    case 0:
                        // 0x4015:
                        // Sound channel enable, DMC Status
                        return this.nes.papu.readReg(address);

                    case 1:
                        // 0x4016:
                        // Joystick 1 + Strobe
                        return this.joy1Read();

                    case 2:
                        // 0x4017:
                        // Joystick 2 + Strobe
                        if (this.mousePressed) {

                            // Check for white pixel nearby:
                            var sx = Math.max(0, this.mouseX - 4);
                            var ex = Math.min(256, this.mouseX + 4);
                            var sy = Math.max(0, this.mouseY - 4);
                            var ey = Math.min(240, this.mouseY + 4);
                            var w = 0;

                            for (var y=sy; y<ey; y++) {
                                for (var x=sx; x<ex; x++) {

                                    if (this.nes.ppu.buffer[(y<<8)+x] == 0xFFFFFF) {
                                        w |= 0x1<<3;
                                        console.debug("Clicked on white!");
                                        break;
                                    }
                                }
                            }

                            w |= (this.mousePressed?(0x1<<4):0);
                            return (this.joy2Read()|w) & 0xFFFF;
                        }
                        else {
                            return this.joy2Read();
                        }

                }
                break;
        }
        return 0;
    },

    regWrite: function(address, value) {
        switch (address) {
            case 0x2000:
                // PPU Control register 1
                this.nes.cpu.mem[address] = value;
                this.nes.ppu.updateControlReg1(value);
                break;

            case 0x2001:
                // PPU Control register 2
                this.nes.cpu.mem[address] = value;
                this.nes.ppu.updateControlReg2(value);
                break;

            case 0x2003:
                // Set Sprite RAM address:
                this.nes.ppu.writeSRAMAddress(value);
                break;

            case 0x2004:
                // Write to Sprite RAM:
                this.nes.ppu.sramWrite(value);
                break;

            case 0x2005:
                // Screen Scroll offsets:
                this.nes.ppu.scrollWrite(value);
                break;

            case 0x2006:
                // Set VRAM address:
                this.nes.ppu.writeVRAMAddress(value);
                break;

            case 0x2007:
                // Write to VRAM:
                this.nes.ppu.vramWrite(value);
                break;

            case 0x4014:
                // Sprite Memory DMA Access
                this.nes.ppu.sramDMA(value);
                break;

            case 0x4015:
                // Sound Channel Switch, DMC Status
                this.nes.papu.writeReg(address, value);
                break;

            case 0x4016:
                // Joystick 1 + Strobe
                if ((value&1) === 0 && (this.joypadLastWrite&1) === 1) {
                    this.joy1StrobeState = 0;
                    this.joy2StrobeState = 0;
                }
                this.joypadLastWrite = value;
                break;

            case 0x4017:
                // Sound channel frame sequencer:
                this.nes.papu.writeReg(address, value);
                break;

            default:
                // Sound registers
                // console.log("write to sound reg");
                if (address >= 0x4000 && address <= 0x4017) {
                    this.nes.papu.writeReg(address,value);
                }

        }
    },

    joy1Read: function() {
        var ret;

        switch (this.joy1StrobeState) {
            case 0:
            case 1:
            case 2:
            case 3:
            case 4:
            case 5:
            case 6:
            case 7:
                ret = this.nes.keyboard.state1[this.joy1StrobeState];
                break;
            case 8:
            case 9:
            case 10:
            case 11:
            case 12:
            case 13:
            case 14:
            case 15:
            case 16:
            case 17:
            case 18:
                ret = 0;
                break;
            case 19:
                ret = 1;
                break;
            default:
                ret = 0;
        }

        this.joy1StrobeState++;
        if (this.joy1StrobeState == 24) {
            this.joy1StrobeState = 0;
        }

        return ret;
    },

    joy2Read: function() {
        var ret;

        switch (this.joy2StrobeState) {
            case 0:
            case 1:
            case 2:
            case 3:
            case 4:
            case 5:
            case 6:
            case 7:
                ret = this.nes.keyboard.state2[this.joy2StrobeState];
                break;
            case 8:
            case 9:
            case 10:
            case 11:
            case 12:
            case 13:
            case 14:
            case 15:
            case 16:
            case 17:
            case 18:
                ret = 0;
                break;
            case 19:
                ret = 1;
                break;
            default:
                ret = 0;
        }

        this.joy2StrobeState++;
        if (this.joy2StrobeState == 24) {
            this.joy2StrobeState = 0;
        }

        return ret;
      },

    loadROM: function() {
        if (!this.nes.rom.valid || this.nes.rom.romCount < 1) {
            alert("NoMapper: Invalid ROM! Unable to load.");
            return;
        }

        // Load ROM into memory:
        this.loadPRGROM();

        // Load CHR-ROM:
        this.loadCHRROM();

        // Load Battery RAM (if present):
        this.loadBatteryRam();

        // Reset IRQ:
        //nes.getCpu().doResetInterrupt();
        this.nes.cpu.requestIrq(this.nes.cpu.IRQ_RESET);
    },

    loadPRGROM: function() {
        if (this.nes.rom.romCount > 1) {
            // Load the two first banks into memory.
            this.loadRomBank(0, 0x8000);
            this.loadRomBank(1, 0xC000);
        }
        else {
            // Load the one bank into both memory locations:
            this.loadRomBank(0, 0x8000);
            this.loadRomBank(0, 0xC000);
        }
    },

    loadCHRROM: function() {
        // console.log("Loading CHR ROM..");
        if (this.nes.rom.vromCount > 0) {
            if (this.nes.rom.vromCount == 1) {
                this.loadVromBank(0,0x0000);
                this.loadVromBank(0,0x1000);
            }
            else {
                this.loadVromBank(0,0x0000);
                this.loadVromBank(1,0x1000);
            }
        }
        else {
            //System.out.println("There aren't any CHR-ROM banks..");
        }
    },

    loadBatteryRam: function() {
        if (this.nes.rom.batteryRam) {
            var ram = this.nes.rom.batteryRam;
            if (ram !== null && ram.length == 0x2000) {
                // Load Battery RAM into memory:
                JSNES.Utils.copyArrayElements(ram, 0, this.nes.cpu.mem, 0x6000, 0x2000);
            }
        }
    },

    loadRomBank: function(bank, address) {
        // Loads a ROM bank into the specified address.
        bank %= this.nes.rom.romCount;
        //var data = this.nes.rom.rom[bank];
        //cpuMem.write(address,data,data.length);
        JSNES.Utils.copyArrayElements(this.nes.rom.rom[bank], 0, this.nes.cpu.mem, address, 16384);
    },

    loadVromBank: function(bank, address) {
        if (this.nes.rom.vromCount === 0) {
            return;
        }
        this.nes.ppu.triggerRendering();

        JSNES.Utils.copyArrayElements(this.nes.rom.vrom[bank % this.nes.rom.vromCount],
            0, this.nes.ppu.vramMem, address, 4096);

        var vromTile = this.nes.rom.vromTile[bank % this.nes.rom.vromCount];
        JSNES.Utils.copyArrayElements(vromTile, 0, this.nes.ppu.ptTile,address >> 4, 256);
    },

    load32kRomBank: function(bank, address) {
        this.loadRomBank((bank*2) % this.nes.rom.romCount, address);
        this.loadRomBank((bank*2+1) % this.nes.rom.romCount, address+16384);
    },

    load8kVromBank: function(bank4kStart, address) {
        if (this.nes.rom.vromCount === 0) {
            return;
        }
        this.nes.ppu.triggerRendering();

        this.loadVromBank((bank4kStart) % this.nes.rom.vromCount, address);
        this.loadVromBank((bank4kStart + 1) % this.nes.rom.vromCount,
                address + 4096);
    },

    load1kVromBank: function(bank1k, address) {
        if (this.nes.rom.vromCount === 0) {
            return;
        }
        this.nes.ppu.triggerRendering();

        var bank4k = Math.floor(bank1k / 4) % this.nes.rom.vromCount;
        var bankoffset = (bank1k % 4) * 1024;
        JSNES.Utils.copyArrayElements(this.nes.rom.vrom[bank4k], 0,
            this.nes.ppu.vramMem, bankoffset, 1024);

        // Update tiles:
        var vromTile = this.nes.rom.vromTile[bank4k];
        var baseIndex = address >> 4;
        for (var i = 0; i < 64; i++) {
            this.nes.ppu.ptTile[baseIndex+i] = vromTile[((bank1k%4) << 6) + i];
        }
    },

    load2kVromBank: function(bank2k, address) {
        if (this.nes.rom.vromCount === 0) {
            return;
        }
        this.nes.ppu.triggerRendering();

        var bank4k = Math.floor(bank2k / 2) % this.nes.rom.vromCount;
        var bankoffset = (bank2k % 2) * 2048;
        JSNES.Utils.copyArrayElements(this.nes.rom.vrom[bank4k], bankoffset,
            this.nes.ppu.vramMem, address, 2048);

        // Update tiles:
        var vromTile = this.nes.rom.vromTile[bank4k];
        var baseIndex = address >> 4;
        for (var i = 0; i < 128; i++) {
            this.nes.ppu.ptTile[baseIndex+i] = vromTile[((bank2k%2) << 7) + i];
        }
    },

    load8kRomBank: function(bank8k, address) {
        var bank16k = Math.floor(bank8k / 2) % this.nes.rom.romCount;
        var offset = (bank8k % 2) * 8192;

        //this.nes.cpu.mem.write(address,this.nes.rom.rom[bank16k],offset,8192);
        JSNES.Utils.copyArrayElements(this.nes.rom.rom[bank16k], offset,
                  this.nes.cpu.mem, address, 8192);
    },

    clockIrqCounter: function() {
        // Does nothing. This is used by the MMC3 mapper.
    },

    latchAccess: function(address) {
        // Does nothing. This is used by MMC2.
    },

    toJSON: function() {
        return {
            'joy1StrobeState': this.joy1StrobeState,
            'joy2StrobeState': this.joy2StrobeState,
            'joypadLastWrite': this.joypadLastWrite
        };
    },

    fromJSON: function(s) {
        this.joy1StrobeState = s.joy1StrobeState;
        this.joy2StrobeState = s.joy2StrobeState;
        this.joypadLastWrite = s.joypadLastWrite;
    }
};


JSNES.Mappers[1] = function(nes) {
    this.nes = nes;
};

JSNES.Mappers[1].prototype = new JSNES.Mappers[0]();

JSNES.Mappers[1].prototype.reset = function() {
    JSNES.Mappers[0].prototype.reset.apply(this);

    // 5-bit buffer:
    this.regBuffer = 0;
    this.regBufferCounter = 0;

    // Register 0:
    this.mirroring = 0;
    this.oneScreenMirroring = 0;
    this.prgSwitchingArea = 1;
    this.prgSwitchingSize = 1;
    this.vromSwitchingSize = 0;

    // Register 1:
    this.romSelectionReg0 = 0;

    // Register 2:
    this.romSelectionReg1 = 0;

    // Register 3:
    this.romBankSelect = 0;
};

JSNES.Mappers[1].prototype.write = function(address, value) {
    // Writes to addresses other than MMC registers are handled by NoMapper.
    if (address < 0x8000) {
        JSNES.Mappers[0].prototype.write.apply(this, arguments);
        return;
    }

    // See what should be done with the written value:
    if ((value & 128) !== 0) {

        // Reset buffering:
        this.regBufferCounter = 0;
        this.regBuffer = 0;

        // Reset register:
        if (this.getRegNumber(address) === 0) {

            this.prgSwitchingArea = 1;
            this.prgSwitchingSize = 1;

        }
    }
    else {

        // Continue buffering:
        //regBuffer = (regBuffer & (0xFF-(1<<regBufferCounter))) | ((value & (1<<regBufferCounter))<<regBufferCounter);
        this.regBuffer = (this.regBuffer & (0xFF - (1 << this.regBufferCounter))) | ((value & 1) << this.regBufferCounter);
        this.regBufferCounter++;

        if (this.regBufferCounter == 5) {
            // Use the buffered value:
            this.setReg(this.getRegNumber(address), this.regBuffer);

            // Reset buffer:
            this.regBuffer = 0;
            this.regBufferCounter = 0;
        }
    }
};

JSNES.Mappers[1].prototype.setReg = function(reg, value) {
    var tmp;

    switch (reg) {
        case 0:
            // Mirroring:
            tmp = value & 3;
            if (tmp !== this.mirroring) {
                // Set mirroring:
                this.mirroring = tmp;
                if ((this.mirroring & 2) === 0) {
                    // SingleScreen mirroring overrides the other setting:
                    this.nes.ppu.setMirroring(
                        this.nes.rom.SINGLESCREEN_MIRRORING);
                }
                // Not overridden by SingleScreen mirroring.
                else if ((this.mirroring & 1) !== 0) {
                    this.nes.ppu.setMirroring(
                        this.nes.rom.HORIZONTAL_MIRRORING
                    );
                }
                else {
                    this.nes.ppu.setMirroring(this.nes.rom.VERTICAL_MIRRORING);
                }
            }

            // PRG Switching Area;
            this.prgSwitchingArea = (value >> 2) & 1;

            // PRG Switching Size:
            this.prgSwitchingSize = (value >> 3) & 1;

            // VROM Switching Size:
            this.vromSwitchingSize = (value >> 4) & 1;

            break;

        case 1:
            // ROM selection:
            this.romSelectionReg0 = (value >> 4) & 1;

            // Check whether the cart has VROM:
            if (this.nes.rom.vromCount > 0) {

                // Select VROM bank at 0x0000:
                if (this.vromSwitchingSize === 0) {

                    // Swap 8kB VROM:
                    if (this.romSelectionReg0 === 0) {
                        this.load8kVromBank((value & 0xF), 0x0000);
                    }
                    else {
                        this.load8kVromBank(
                            Math.floor(this.nes.rom.vromCount / 2) +
                                (value & 0xF),
                            0x0000
                        );
                    }

                }
                else {
                    // Swap 4kB VROM:
                    if (this.romSelectionReg0 === 0) {
                        this.loadVromBank((value & 0xF), 0x0000);
                    }
                    else {
                        this.loadVromBank(
                            Math.floor(this.nes.rom.vromCount / 2) +
                                (value & 0xF),
                            0x0000
                        );
                    }
                }
            }

            break;

        case 2:
            // ROM selection:
            this.romSelectionReg1 = (value >> 4) & 1;

            // Check whether the cart has VROM:
            if (this.nes.rom.vromCount > 0) {

                // Select VROM bank at 0x1000:
                if (this.vromSwitchingSize === 1) {
                    // Swap 4kB of VROM:
                    if (this.romSelectionReg1 === 0) {
                        this.loadVromBank((value & 0xF), 0x1000);
                    }
                    else {
                        this.loadVromBank(
                            Math.floor(this.nes.rom.vromCount / 2) +
                                (value & 0xF),
                            0x1000
                        );
                    }
                }
            }
            break;

        default:
            // Select ROM bank:
            // -------------------------
            tmp = value & 0xF;
            var bank;
            var baseBank = 0;

            if (this.nes.rom.romCount >= 32) {
                // 1024 kB cart
                if (this.vromSwitchingSize === 0) {
                    if (this.romSelectionReg0 === 1) {
                        baseBank = 16;
                    }
                }
                else {
                    baseBank = (this.romSelectionReg0
                                | (this.romSelectionReg1 << 1)) << 3;
                }
            }
            else if (this.nes.rom.romCount >= 16) {
                // 512 kB cart
                if (this.romSelectionReg0 === 1) {
                    baseBank = 8;
                }
            }

            if (this.prgSwitchingSize === 0) {
                // 32kB
                bank = baseBank + (value & 0xF);
                this.load32kRomBank(bank, 0x8000);
            }
            else {
                // 16kB
                bank = baseBank * 2 + (value & 0xF);
                if (this.prgSwitchingArea === 0) {
                    this.loadRomBank(bank, 0xC000);
                }
                else {
                    this.loadRomBank(bank, 0x8000);
                }
            }
    }
};

// Returns the register number from the address written to:
JSNES.Mappers[1].prototype.getRegNumber = function(address) {
    if (address >= 0x8000 && address <= 0x9FFF) {
        return 0;
    }
    else if (address >= 0xA000 && address <= 0xBFFF) {
        return 1;
    }
    else if (address >= 0xC000 && address <= 0xDFFF) {
        return 2;
    }
    else {
        return 3;
    }
};

JSNES.Mappers[1].prototype.loadROM = function(rom) {
    if (!this.nes.rom.valid) {
        alert("MMC1: Invalid ROM! Unable to load.");
        return;
    }

    // Load PRG-ROM:
    this.loadRomBank(0, 0x8000);                         //   First ROM bank..
    this.loadRomBank(this.nes.rom.romCount - 1, 0xC000); // ..and last ROM bank.

    // Load CHR-ROM:
    this.loadCHRROM();

    // Load Battery RAM (if present):
    this.loadBatteryRam();

    // Do Reset-Interrupt:
    this.nes.cpu.requestIrq(this.nes.cpu.IRQ_RESET);
};

JSNES.Mappers[1].prototype.switchLowHighPrgRom = function(oldSetting) {
    // not yet.
};

JSNES.Mappers[1].prototype.switch16to32 = function() {
    // not yet.
};

JSNES.Mappers[1].prototype.switch32to16 = function() {
    // not yet.
};

JSNES.Mappers[1].prototype.toJSON = function() {
    var s = JSNES.Mappers[0].prototype.toJSON.apply(this);
    s.mirroring = this.mirroring;
    s.oneScreenMirroring = this.oneScreenMirroring;
    s.prgSwitchingArea = this.prgSwitchingArea;
    s.prgSwitchingSize = this.prgSwitchingSize;
    s.vromSwitchingSize = this.vromSwitchingSize;
    s.romSelectionReg0 = this.romSelectionReg0;
    s.romSelectionReg1 = this.romSelectionReg1;
    s.romBankSelect = this.romBankSelect;
    s.regBuffer = this.regBuffer;
    s.regBufferCounter = this.regBufferCounter;
    return s;
};

JSNES.Mappers[1].prototype.fromJSON = function(s) {
    JSNES.Mappers[0].prototype.fromJSON.apply(this, s);
    this.mirroring = s.mirroring;
    this.oneScreenMirroring = s.oneScreenMirroring;
    this.prgSwitchingArea = s.prgSwitchingArea;
    this.prgSwitchingSize = s.prgSwitchingSize;
    this.vromSwitchingSize = s.vromSwitchingSize;
    this.romSelectionReg0 = s.romSelectionReg0;
    this.romSelectionReg1 = s.romSelectionReg1;
    this.romBankSelect = s.romBankSelect;
    this.regBuffer = s.regBuffer;
    this.regBufferCounter = s.regBufferCounter;
};

JSNES.Mappers[2] = function(nes) {
    this.nes = nes;
};

JSNES.Mappers[2].prototype = new JSNES.Mappers[0]();

JSNES.Mappers[2].prototype.write = function(address, value) {
    // Writes to addresses other than MMC registers are handled by NoMapper.
    if (address < 0x8000) {
        JSNES.Mappers[0].prototype.write.apply(this, arguments);
        return;
    }

    else {
        // This is a ROM bank select command.
        // Swap in the given ROM bank at 0x8000:
        this.loadRomBank(value, 0x8000);
    }
};

JSNES.Mappers[2].prototype.loadROM = function(rom) {
    if (!this.nes.rom.valid) {
        alert("UNROM: Invalid ROM! Unable to load.");
        return;
    }

    // Load PRG-ROM:
    this.loadRomBank(0, 0x8000);
    this.loadRomBank(this.nes.rom.romCount - 1, 0xC000);

    // Load CHR-ROM:
    this.loadCHRROM();

    // Do Reset-Interrupt:
    this.nes.cpu.requestIrq(this.nes.cpu.IRQ_RESET);
};


JSNES.Mappers[4] = function(nes) {
    this.nes = nes;

    this.CMD_SEL_2_1K_VROM_0000 = 0;
    this.CMD_SEL_2_1K_VROM_0800 = 1;
    this.CMD_SEL_1K_VROM_1000 = 2;
    this.CMD_SEL_1K_VROM_1400 = 3;
    this.CMD_SEL_1K_VROM_1800 = 4;
    this.CMD_SEL_1K_VROM_1C00 = 5;
    this.CMD_SEL_ROM_PAGE1 = 6;
    this.CMD_SEL_ROM_PAGE2 = 7;

    this.command = null;
    this.prgAddressSelect = null;
    this.chrAddressSelect = null;
    this.pageNumber = null;
    this.irqCounter = null;
    this.irqLatchValue = null;
    this.irqEnable = null;
    this.prgAddressChanged = false;
};

JSNES.Mappers[4].prototype = new JSNES.Mappers[0]();

JSNES.Mappers[4].prototype.write = function(address, value) {
    // Writes to addresses other than MMC registers are handled by NoMapper.
    if (address < 0x8000) {
        JSNES.Mappers[0].prototype.write.apply(this, arguments);
        return;
    }

    switch (address) {
        case 0x8000:
            // Command/Address Select register
            this.command = value & 7;
            var tmp = (value >> 6) & 1;
            if (tmp != this.prgAddressSelect) {
                this.prgAddressChanged = true;
            }
            this.prgAddressSelect = tmp;
            this.chrAddressSelect = (value >> 7) & 1;
            break;

        case 0x8001:
            // Page number for command
            this.executeCommand(this.command, value);
            break;

        case 0xA000:
            // Mirroring select
            if ((value & 1) !== 0) {
                this.nes.ppu.setMirroring(
                    this.nes.rom.HORIZONTAL_MIRRORING
                );
            }
            else {
                this.nes.ppu.setMirroring(this.nes.rom.VERTICAL_MIRRORING);
            }
            break;

        case 0xA001:
            // SaveRAM Toggle
            // TODO
            //nes.getRom().setSaveState((value&1)!=0);
            break;

        case 0xC000:
            // IRQ Counter register
            this.irqCounter = value;
            //nes.ppu.mapperIrqCounter = 0;
            break;

        case 0xC001:
            // IRQ Latch register
            this.irqLatchValue = value;
            break;

        case 0xE000:
            // IRQ Control Reg 0 (disable)
            //irqCounter = irqLatchValue;
            this.irqEnable = 0;
            break;

        case 0xE001:
            // IRQ Control Reg 1 (enable)
            this.irqEnable = 1;
            break;

        default:
            // Not a MMC3 register.
            // The game has probably crashed,
            // since it tries to write to ROM..
            // IGNORE.
    }
};

JSNES.Mappers[4].prototype.executeCommand = function(cmd, arg) {
    switch (cmd) {
        case this.CMD_SEL_2_1K_VROM_0000:
            // Select 2 1KB VROM pages at 0x0000:
            if (this.chrAddressSelect === 0) {
                this.load1kVromBank(arg, 0x0000);
                this.load1kVromBank(arg + 1, 0x0400);
            }
            else {
                this.load1kVromBank(arg, 0x1000);
                this.load1kVromBank(arg + 1, 0x1400);
            }
            break;

        case this.CMD_SEL_2_1K_VROM_0800:
            // Select 2 1KB VROM pages at 0x0800:
            if (this.chrAddressSelect === 0) {
                this.load1kVromBank(arg, 0x0800);
                this.load1kVromBank(arg + 1, 0x0C00);
            }
            else {
                this.load1kVromBank(arg, 0x1800);
                this.load1kVromBank(arg + 1, 0x1C00);
            }
            break;

        case this.CMD_SEL_1K_VROM_1000:
            // Select 1K VROM Page at 0x1000:
            if (this.chrAddressSelect === 0) {
                this.load1kVromBank(arg, 0x1000);
            }
            else {
                this.load1kVromBank(arg, 0x0000);
            }
            break;

        case this.CMD_SEL_1K_VROM_1400:
            // Select 1K VROM Page at 0x1400:
            if (this.chrAddressSelect === 0) {
                this.load1kVromBank(arg, 0x1400);
            }
            else {
                this.load1kVromBank(arg, 0x0400);
            }
            break;

        case this.CMD_SEL_1K_VROM_1800:
            // Select 1K VROM Page at 0x1800:
            if (this.chrAddressSelect === 0) {
                this.load1kVromBank(arg, 0x1800);
            }
            else {
                this.load1kVromBank(arg, 0x0800);
            }
            break;

        case this.CMD_SEL_1K_VROM_1C00:
            // Select 1K VROM Page at 0x1C00:
            if (this.chrAddressSelect === 0) {
                this.load1kVromBank(arg, 0x1C00);
            }else {
                this.load1kVromBank(arg, 0x0C00);
            }
            break;

        case this.CMD_SEL_ROM_PAGE1:
            if (this.prgAddressChanged) {
                // Load the two hardwired banks:
                if (this.prgAddressSelect === 0) {
                    this.load8kRomBank(
                        ((this.nes.rom.romCount - 1) * 2),
                        0xC000
                    );
                }
                else {
                    this.load8kRomBank(
                        ((this.nes.rom.romCount - 1) * 2),
                        0x8000
                    );
                }
                this.prgAddressChanged = false;
            }

            // Select first switchable ROM page:
            if (this.prgAddressSelect === 0) {
                this.load8kRomBank(arg, 0x8000);
            }
            else {
                this.load8kRomBank(arg, 0xC000);
            }
            break;

        case this.CMD_SEL_ROM_PAGE2:
            // Select second switchable ROM page:
            this.load8kRomBank(arg, 0xA000);

            // hardwire appropriate bank:
            if (this.prgAddressChanged) {
                // Load the two hardwired banks:
                if (this.prgAddressSelect === 0) {
                    this.load8kRomBank(
                        ((this.nes.rom.romCount - 1) * 2),
                        0xC000
                    );
                }
                else {
                    this.load8kRomBank(
                        ((this.nes.rom.romCount - 1) * 2),
                        0x8000
                    );
                }
                this.prgAddressChanged = false;
            }
    }
};

JSNES.Mappers[4].prototype.loadROM = function(rom) {
    if (!this.nes.rom.valid) {
        alert("MMC3: Invalid ROM! Unable to load.");
        return;
    }

    // Load hardwired PRG banks (0xC000 and 0xE000):
    this.load8kRomBank(((this.nes.rom.romCount - 1) * 2), 0xC000);
    this.load8kRomBank(((this.nes.rom.romCount - 1) * 2) + 1, 0xE000);

    // Load swappable PRG banks (0x8000 and 0xA000):
    this.load8kRomBank(0, 0x8000);
    this.load8kRomBank(1, 0xA000);

    // Load CHR-ROM:
    this.loadCHRROM();

    // Load Battery RAM (if present):
    this.loadBatteryRam();

    // Do Reset-Interrupt:
    this.nes.cpu.requestIrq(this.nes.cpu.IRQ_RESET);
};

JSNES.Mappers[4].prototype.clockIrqCounter = function() {
    if (this.irqEnable == 1) {
        this.irqCounter--;
        if (this.irqCounter < 0) {
            // Trigger IRQ:
            //nes.getCpu().doIrq();
            this.nes.cpu.requestIrq(this.nes.cpu.IRQ_NORMAL);
            this.irqCounter = this.irqLatchValue;
        }
    }
};

JSNES.Mappers[4].prototype.toJSON = function() {
    var s = JSNES.Mappers[0].prototype.toJSON.apply(this);
    s.command = this.command;
    s.prgAddressSelect = this.prgAddressSelect;
    s.chrAddressSelect = this.chrAddressSelect;
    s.pageNumber = this.pageNumber;
    s.irqCounter = this.irqCounter;
    s.irqLatchValue = this.irqLatchValue;
    s.irqEnable = this.irqEnable;
    s.prgAddressChanged = this.prgAddressChanged;
    return s;
};

JSNES.Mappers[4].prototype.fromJSON = function(s) {
    JSNES.Mappers[0].prototype.fromJSON.apply(this, s);
    this.command = s.command;
    this.prgAddressSelect = s.prgAddressSelect;
    this.chrAddressSelect = s.chrAddressSelect;
    this.pageNumber = s.pageNumber;
    this.irqCounter = s.irqCounter;
    this.irqLatchValue = s.irqLatchValue;
    this.irqEnable = s.irqEnable;
    this.prgAddressChanged = s.prgAddressChanged;
};


/**
 * Mapper005 (MMC5,ExROM)
 *
 * @example Castlevania 3, Just Breed, Uncharted Waters, Romance of the 3 Kingdoms 2, Laser Invasion, Metal Slader Glory, Uchuu Keibitai SDF, Shin 4 Nin Uchi Mahjong - Yakuman Tengoku
 * @description http://wiki.nesdev.com/w/index.php/INES_Mapper_005
 * @constructor
 */
JSNES.Mappers[5] = function(nes) {
    this.nes = nes;
};

JSNES.Mappers[5].prototype = new JSNES.Mappers[0]();

JSNES.Mappers[5].prototype.write = function(address, value) {
    // Writes to addresses other than MMC registers are handled by NoMapper.
    if (address < 0x8000) {
        JSNES.Mappers[0].prototype.write.apply(this, arguments);
    } else {
        this.load8kVromBank(value, 0x0000);
    }
};

JSNES.Mappers[5].prototype.write = function(address, value) {
    // Writes to addresses other than MMC registers are handled by NoMapper.
    if (address < 0x5000) {
        JSNES.Mappers[0].prototype.write.apply(this, arguments);
        return;
    }

    switch(address) {
        case 0x5100: this.prg_size = value & 3; break;
        case 0x5101: this.chr_size = value & 3; break;
        case 0x5102: this.sram_we_a = value & 3; break;
        case 0x5103: this.sram_we_b = value & 3; break;
        case 0x5104: this.graphic_mode = value & 3; break;
        case 0x5105:
            this.nametable_mode = value;
            this.nametable_type[0] = value & 3;
            this.load1kVromBank(value & 3, 0x2000);
            value >>= 2;
            this.nametable_type[1] = value & 3;
            this.load1kVromBank(value & 3, 0x2400);
            value >>= 2;
            this.nametable_type[2] = value & 3;
            this.load1kVromBank(value & 3, 0x2800);
            value >>= 2;
            this.nametable_type[3] = value & 3;
            this.load1kVromBank(value & 3, 0x2C00);
            break;
        case 0x5106: this.fill_chr = value; break;
        case 0x5107: this.fill_pal = value & 3; break;
        case 0x5113: this.SetBank_SRAM(3, value & 3); break;
        case 0x5114:
        case 0x5115:
        case 0x5116:
        case 0x5117:
            this.SetBank_CPU(address, value);
            break;
        case 0x5120:
        case 0x5121:
        case 0x5122:
        case 0x5123:
        case 0x5124:
        case 0x5125:
        case 0x5126:
        case 0x5127:
            this.chr_mode = 0;
            this.chr_page[0][address&7] = value;
            this.SetBank_PPU();
            break;
        case 0x5128:
        case 0x5129:
        case 0x512A:
        case 0x512B:
            this.chr_mode = 1;
            this.chr_page[1][(address&3)+0] = value;
            this.chr_page[1][(address&3)+4] = value;
            this.SetBank_PPU();
            break;
        case 0x5200: this.split_control = value; break;
        case 0x5201: this.split_scroll = value; break;
        case 0x5202: this.split_page = value & 0x3F; break;
        case 0x5203:
            this.irq_line = value;
            this.nes.cpu.ClearIRQ();
            break;
        case 0x5204:
            this.irq_enable = value;
            this.nes.cpu.ClearIRQ();
            break;
        case 0x5205: this.mult_a = value; break;
        case 0x5206: this.mult_b = value; break;
        default:
            if(address >= 0x5000 && address <= 0x5015) {
                this.nes.papu.exWrite(address, value);
            } else if(address >= 0x5C00 && address <= 0x5FFF) {
                if(this.graphic_mode == 2) { 			// ExRAM
                    // vram write
                } else if(this.graphic_mode != 3) {		// Split,ExGraphic
                    if(this.irq_status & 0x40) {
                        // vram write
                    } else {
                        // vram write
                    }
                }
            } else if(address >= 0x6000 && address <= 0x7FFF) {
                if(this.sram_we_a == 2 && this.sram_we_b == 1) {
                    // additional ram write
                }
            }
            break;
    }
};

JSNES.Mappers[5].prototype.loadROM = function(rom) {
    if (!this.nes.rom.valid) {
        alert("UNROM: Invalid ROM! Unable to load.");
        return;
    }

    // Load PRG-ROM:
    this.load8kRomBank(this.nes.rom.romCount * 2 - 1, 0x8000);
    this.load8kRomBank(this.nes.rom.romCount * 2 - 1, 0xA000);
    this.load8kRomBank(this.nes.rom.romCount * 2 - 1, 0xC000);
    this.load8kRomBank(this.nes.rom.romCount * 2 - 1, 0xE000);

    // Load CHR-ROM:
    this.loadCHRROM();

    // Do Reset-Interrupt:
    this.nes.cpu.requestIrq(this.nes.cpu.IRQ_RESET);
};


/**
 * Mapper007 (AxROM)
 * @example Battletoads, Time Lord, Marble Madness
 * @description http://wiki.nesdev.com/w/index.php/INES_Mapper_007
 * @constructor
 */
JSNES.Mappers[7] = function(nes) {
    this.nes = nes;
};

JSNES.Mappers[7].prototype = new JSNES.Mappers[0]();

JSNES.Mappers[7].prototype.write = function(address, value) {
    // Writes to addresses other than MMC registers are handled by NoMapper.
    if (address < 0x8000) {
        JSNES.Mappers[0].prototype.write.apply(this, arguments);
    } else {
        this.load32kRomBank(value & 0x7, 0x8000);
        if(value & 0x10) {
            this.nes.ppu.setMirroring(this.nes.rom.SINGLESCREEN_MIRRORING2);
        } else {
            this.nes.ppu.setMirroring(this.nes.rom.SINGLESCREEN_MIRRORING);
        }
    }
};

JSNES.Mappers[7].prototype.loadROM = function(rom) {
    if (!this.nes.rom.valid) {
        alert("AOROM: Invalid ROM! Unable to load.");
        return;
    }

    // Load PRG-ROM:
    this.loadPRGROM();

    // Load CHR-ROM:
    this.loadCHRROM();

    // Do Reset-Interrupt:
    this.nes.cpu.requestIrq(this.nes.cpu.IRQ_RESET);
};

/**
 * Mapper 011 (Color Dreams)
 *
 * @description http://wiki.nesdev.com/w/index.php/Color_Dreams
 * @example Crystal Mines, Metal Fighter
 * @constructor
 */
JSNES.Mappers[11] = function(nes) {
    this.nes = nes;
};

JSNES.Mappers[11].prototype = new JSNES.Mappers[0]();

JSNES.Mappers[11].prototype.write = function(address, value) {
    if (address < 0x8000) {
        JSNES.Mappers[0].prototype.write.apply(this, arguments);
        return;
    } else {
        // Swap in the given PRG-ROM bank:
        var prgbank1 = ((value & 0xF) * 2) % this.nes.rom.romCount;
        var prgbank2 = ((value & 0xF) * 2 + 1) % this.nes.rom.romCount;

        this.loadRomBank(prgbank1, 0x8000);
        this.loadRomBank(prgbank2, 0xC000);


        if (this.nes.rom.vromCount > 0) {
            // Swap in the given VROM bank at 0x0000:
            var bank = ((value >> 4) * 2) % (this.nes.rom.vromCount);
            this.loadVromBank(bank, 0x0000);
            this.loadVromBank(bank + 1, 0x1000);
        }
    }
};

/**
 * Mapper 034 (BNROM, NINA-01)
 *
 * @description http://wiki.nesdev.com/w/index.php/INES_Mapper_034
 * @example Darkseed, Mashou, Mission Impossible 2
 * @constructor
 */
JSNES.Mappers[34] = function(nes) {
    this.nes = nes;
};

JSNES.Mappers[34].prototype = new JSNES.Mappers[0]();

JSNES.Mappers[34].prototype.write = function(address, value) {
    if (address < 0x8000) {
        JSNES.Mappers[0].prototype.write.apply(this, arguments);
        return;
    } else {
        this.load32kRomBank(value, 0x8000);
    }
};



/**
 * Mapper 066 (GxROM)
 *
 * @description http://wiki.nesdev.com/w/index.php/INES_Mapper_066
 * @example Doraemon, Dragon Power, Gumshoe, Thunder & Lightning,
 * Super Mario Bros. + Duck Hunt
 * @constructor
 */
JSNES.Mappers[66] = function(nes) {
    this.nes = nes;
    console.log('Mapper 66');
};

JSNES.Mappers[66].prototype = new JSNES.Mappers[0]();

JSNES.Mappers[66].prototype.write = function(address, value) {
    if (address < 0x8000) {
        JSNES.Mappers[0].prototype.write.apply(this, arguments);
        return;
    } else {
        // Swap in the given PRG-ROM bank at 0x8000:
        this.load32kRomBank((value >> 4) & 3, 0x8000);

        // Swap in the given VROM bank at 0x0000:
        this.load8kVromBank((value & 3) * 2, 0x0000);
    }
};
/*
JSNES, based on Jamie Sanders' vNES
Copyright (C) 2010 Ben Firshman

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

JSNES.PAPU = function(nes) {
    this.nes = nes;

    this.square1 = new JSNES.PAPU.ChannelSquare(this, true);
    this.square2 = new JSNES.PAPU.ChannelSquare(this, false);
    this.triangle = new JSNES.PAPU.ChannelTriangle(this);
    this.noise = new JSNES.PAPU.ChannelNoise(this);
    this.dmc = new JSNES.PAPU.ChannelDM(this);

    this.frameIrqCounter = null;
    this.frameIrqCounterMax = 4;
    this.initCounter = 2048;
    this.channelEnableValue = null;

    this.bufferSize = 8192;
    this.bufferIndex = 0;
    this.sampleRate = 44100;

    this.lengthLookup = null;
    this.dmcFreqLookup = null;
    this.noiseWavelengthLookup = null;
    this.square_table = null;
    this.tnd_table = null;
    this.sampleBuffer = new Array(this.bufferSize*2);

    this.frameIrqEnabled = false;
    this.frameIrqActive = null;
    this.frameClockNow = null;
    this.startedPlaying=false;
    this.recordOutput = false;
    this.initingHardware = false;

    this.masterFrameCounter = null;
    this.derivedFrameCounter = null;
    this.countSequence = null;
    this.sampleTimer = null;
    this.frameTime = null;
    this.sampleTimerMax = null;
    this.sampleCount = null;
    this.triValue = 0;

    this.smpSquare1 = null;
    this.smpSquare2 = null;
    this.smpTriangle = null;
    this.smpDmc = null;
    this.accCount = null;

    // DC removal vars:
    this.prevSampleL = 0;
    this.prevSampleR = 0;
    this.smpAccumL = 0;
    this.smpAccumR = 0;

    // DAC range:
    this.dacRange = 0;
    this.dcValue = 0;

    // Master volume:
    this.masterVolume = 256;

    // Stereo positioning:
    this.stereoPosLSquare1 = null;
    this.stereoPosLSquare2 = null;
    this.stereoPosLTriangle = null;
    this.stereoPosLNoise = null;
    this.stereoPosLDMC = null;
    this.stereoPosRSquare1 = null;
    this.stereoPosRSquare2 = null;
    this.stereoPosRTriangle = null;
    this.stereoPosRNoise = null;
    this.stereoPosRDMC = null;

    this.extraCycles = null;

    this.maxSample = null;
    this.minSample = null;

    // Panning:
    this.panning = [80, 170, 100, 150, 128];
    this.setPanning(this.panning);

    // Initialize lookup tables:
    this.initLengthLookup();
    this.initDmcFrequencyLookup();
    this.initNoiseWavelengthLookup();
    this.initDACtables();

    // Init sound registers:
    for (var i = 0; i < 0x14; i++) {
        if (i === 0x10){
            this.writeReg(0x4010, 0x10);
        }
        else {
            this.writeReg(0x4000 + i, 0);
        }
    }

    this.reset();
};

JSNES.PAPU.prototype = {
    reset: function() {
        this.sampleRate = this.nes.opts.sampleRate;
        this.sampleTimerMax = Math.floor(
            (1024.0 * this.nes.opts.CPU_FREQ_NTSC *
                this.nes.opts.preferredFrameRate) /
                (this.sampleRate * 60.0)
        );

        this.frameTime = Math.floor(
            (14915.0 * this.nes.opts.preferredFrameRate) / 60.0
        );

        this.sampleTimer = 0;
        this.bufferIndex = 0;

        this.updateChannelEnable(0);
        this.masterFrameCounter = 0;
        this.derivedFrameCounter = 0;
        this.countSequence = 0;
        this.sampleCount = 0;
        this.initCounter = 2048;
        this.frameIrqEnabled = false;
        this.initingHardware = false;

        this.resetCounter();

        this.square1.reset();
        this.square2.reset();
        this.triangle.reset();
        this.noise.reset();
        this.dmc.reset();

        this.bufferIndex = 0;
        this.accCount = 0;
        this.smpSquare1 = 0;
        this.smpSquare2 = 0;
        this.smpTriangle = 0;
        this.smpDmc = 0;

        this.frameIrqEnabled = false;
        this.frameIrqCounterMax = 4;

        this.channelEnableValue = 0xFF;
        this.startedPlaying = false;
        this.prevSampleL = 0;
        this.prevSampleR = 0;
        this.smpAccumL = 0;
        this.smpAccumR = 0;

        this.maxSample = -500000;
        this.minSample = 500000;
    },

    readReg: function(address){
        // Read 0x4015:
        var tmp = 0;
        tmp |= (this.square1.getLengthStatus()   );
        tmp |= (this.square2.getLengthStatus() <<1);
        tmp |= (this.triangle.getLengthStatus()<<2);
        tmp |= (this.noise.getLengthStatus()   <<3);
        tmp |= (this.dmc.getLengthStatus()     <<4);
        tmp |= (((this.frameIrqActive && this.frameIrqEnabled)? 1 : 0) << 6);
        tmp |= (this.dmc.getIrqStatus()        <<7);

        this.frameIrqActive = false;
        this.dmc.irqGenerated = false;

        return tmp & 0xFFFF;
    },

    writeReg: function(address, value){
        if (address >= 0x4000 && address < 0x4004) {
            // Square Wave 1 Control
            this.square1.writeReg(address, value);
            // console.log("Square Write");
        }
        else if (address >= 0x4004 && address < 0x4008) {
            // Square 2 Control
            this.square2.writeReg(address, value);
        }
        else if (address >= 0x4008 && address < 0x400C) {
            // Triangle Control
            this.triangle.writeReg(address, value);
        }
        else if (address >= 0x400C && address <= 0x400F) {
            // Noise Control
            this.noise.writeReg(address, value);
        }
        else if (address === 0x4010){
            // DMC Play mode & DMA frequency
            this.dmc.writeReg(address, value);
        }
        else if (address === 0x4011){
            // DMC Delta Counter
            this.dmc.writeReg(address, value);
        }
        else if (address === 0x4012){
            // DMC Play code starting address
            this.dmc.writeReg(address, value);
        }
        else if (address === 0x4013){
            // DMC Play code length
            this.dmc.writeReg(address, value);
        }
        else if (address === 0x4015){
            // Channel enable
            this.updateChannelEnable(value);

            if (value !== 0 && this.initCounter > 0) {
                // Start hardware initialization
                this.initingHardware = true;
            }

            // DMC/IRQ Status
            this.dmc.writeReg(address, value);
        }
        else if (address === 0x4017) {
            // Frame counter control
            this.countSequence = (value>>7)&1;
            this.masterFrameCounter = 0;
            this.frameIrqActive = false;

            if (((value>>6)&0x1)===0){
                this.frameIrqEnabled = true;
            }
            else {
                this.frameIrqEnabled = false;
            }

            if (this.countSequence === 0) {
                // NTSC:
                this.frameIrqCounterMax = 4;
                this.derivedFrameCounter = 4;
            }
            else {
                // PAL:
                this.frameIrqCounterMax = 5;
                this.derivedFrameCounter = 0;
                this.frameCounterTick();
            }
        }
    },

    resetCounter: function(){
        if (this.countSequence === 0) {
            this.derivedFrameCounter = 4;
        }else{
            this.derivedFrameCounter = 0;
        }
    },

    // Updates channel enable status.
    // This is done on writes to the
    // channel enable register (0x4015),
    // and when the user enables/disables channels
    // in the GUI.
    updateChannelEnable: function(value){
        this.channelEnableValue = value&0xFFFF;
        this.square1.setEnabled((value&1) !== 0);
        this.square2.setEnabled((value&2) !== 0);
        this.triangle.setEnabled((value&4) !== 0);
        this.noise.setEnabled((value&8) !== 0);
        this.dmc.setEnabled((value&16) !== 0);
    },

    // Clocks the frame counter. It should be clocked at
    // twice the cpu speed, so the cycles will be
    // divided by 2 for those counters that are
    // clocked at cpu speed.
    clockFrameCounter: function(nCycles){
        if (this.initCounter > 0) {
            if (this.initingHardware) {
                this.initCounter -= nCycles;
                if (this.initCounter <= 0) {
                    this.initingHardware = false;
                }
                return;
            }
        }

        // Don't process ticks beyond next sampling:
        nCycles += this.extraCycles;
        var maxCycles = this.sampleTimerMax-this.sampleTimer;
        if ((nCycles<<10) > maxCycles) {

            this.extraCycles = ((nCycles<<10) - maxCycles)>>10;
            nCycles -= this.extraCycles;

        }else{

            this.extraCycles = 0;

        }

        var dmc = this.dmc;
        var triangle = this.triangle;
        var square1 = this.square1;
        var square2 = this.square2;
        var noise = this.noise;

        // Clock DMC:
        if (dmc.isEnabled) {

            dmc.shiftCounter-=(nCycles<<3);
            while(dmc.shiftCounter<=0 && dmc.dmaFrequency>0){
                dmc.shiftCounter += dmc.dmaFrequency;
                dmc.clockDmc();
            }

        }

        // Clock Triangle channel Prog timer:
        if (triangle.progTimerMax>0) {

            triangle.progTimerCount -= nCycles;
            while(triangle.progTimerCount <= 0){

                triangle.progTimerCount += triangle.progTimerMax+1;
                if (triangle.linearCounter>0 && triangle.lengthCounter>0) {

                    triangle.triangleCounter++;
                    triangle.triangleCounter &= 0x1F;

                    if (triangle.isEnabled) {
                        if (triangle.triangleCounter>=0x10) {
                            // Normal value.
                            triangle.sampleValue = (triangle.triangleCounter&0xF);
                        }else{
                            // Inverted value.
                            triangle.sampleValue = (0xF - (triangle.triangleCounter&0xF));
                        }
                        triangle.sampleValue <<= 4;
                    }
                }
            }
        }

        // Clock Square channel 1 Prog timer:
        square1.progTimerCount -= nCycles;
        if (square1.progTimerCount <= 0) {

            square1.progTimerCount += (square1.progTimerMax+1)<<1;

            square1.squareCounter++;
            square1.squareCounter&=0x7;
            square1.updateSampleValue();

        }

        // Clock Square channel 2 Prog timer:
        square2.progTimerCount -= nCycles;
        if (square2.progTimerCount <= 0) {

            square2.progTimerCount += (square2.progTimerMax+1)<<1;

            square2.squareCounter++;
            square2.squareCounter&=0x7;
            square2.updateSampleValue();

        }

        // Clock noise channel Prog timer:
        var acc_c = nCycles;
        if (noise.progTimerCount-acc_c > 0) {

            // Do all cycles at once:
            noise.progTimerCount -= acc_c;
            noise.accCount       += acc_c;
            noise.accValue       += acc_c * noise.sampleValue;

        }else{

            // Slow-step:
            while((acc_c--) > 0){

                if (--noise.progTimerCount <= 0 && noise.progTimerMax>0) {

                    // Update noise shift register:
                    noise.shiftReg <<= 1;
                    noise.tmp = (((noise.shiftReg << (noise.randomMode===0?1:6)) ^ noise.shiftReg) & 0x8000 );
                    if (noise.tmp !== 0) {

                        // Sample value must be 0.
                        noise.shiftReg |= 0x01;
                        noise.randomBit = 0;
                        noise.sampleValue = 0;

                    }else{

                        // Find sample value:
                        noise.randomBit = 1;
                        if (noise.isEnabled && noise.lengthCounter>0) {
                            noise.sampleValue = noise.masterVolume;
                        }else{
                            noise.sampleValue = 0;
                        }

                    }

                    noise.progTimerCount += noise.progTimerMax;

                }

                noise.accValue += noise.sampleValue;
                noise.accCount++;

            }
        }


        // Frame IRQ handling:
        if (this.frameIrqEnabled && this.frameIrqActive){
            this.nes.cpu.requestIrq(this.nes.cpu.IRQ_NORMAL);
        }

        // Clock frame counter at double CPU speed:
        this.masterFrameCounter += (nCycles<<1);
        if (this.masterFrameCounter >= this.frameTime) {
            // 240Hz tick:
            this.masterFrameCounter -= this.frameTime;
            this.frameCounterTick();
        }

        // Accumulate sample value:
        this.accSample(nCycles);

        // Clock sample timer:
        this.sampleTimer += nCycles<<10;
        if (this.sampleTimer>=this.sampleTimerMax) {
            // Sample channels:
            this.sample();
            this.sampleTimer -= this.sampleTimerMax;
        }
    },

    accSample: function(cycles) {
        // Special treatment for triangle channel - need to interpolate.
        if (this.triangle.sampleCondition) {
            this.triValue = Math.floor((this.triangle.progTimerCount << 4) /
                    (this.triangle.progTimerMax + 1));
            if (this.triValue > 16) {
                this.triValue = 16;
            }
            if (this.triangle.triangleCounter >= 16) {
                this.triValue = 16 - this.triValue;
            }

            // Add non-interpolated sample value:
            this.triValue += this.triangle.sampleValue;
        }

        // Now sample normally:
        if (cycles === 2) {

            this.smpTriangle += this.triValue                << 1;
            this.smpDmc      += this.dmc.sample         << 1;
            this.smpSquare1  += this.square1.sampleValue    << 1;
            this.smpSquare2  += this.square2.sampleValue    << 1;
            this.accCount    += 2;

        }else if (cycles === 4) {

            this.smpTriangle += this.triValue                << 2;
            this.smpDmc      += this.dmc.sample         << 2;
            this.smpSquare1  += this.square1.sampleValue    << 2;
            this.smpSquare2  += this.square2.sampleValue    << 2;
            this.accCount    += 4;

        }else{

            this.smpTriangle += cycles * this.triValue;
            this.smpDmc      += cycles * this.dmc.sample;
            this.smpSquare1  += cycles * this.square1.sampleValue;
            this.smpSquare2  += cycles * this.square2.sampleValue;
            this.accCount    += cycles;

        }

    },

    frameCounterTick: function(){

        this.derivedFrameCounter++;
        if (this.derivedFrameCounter >= this.frameIrqCounterMax) {
            this.derivedFrameCounter = 0;
        }

        if (this.derivedFrameCounter===1 || this.derivedFrameCounter===3) {

            // Clock length & sweep:
            this.triangle.clockLengthCounter();
            this.square1.clockLengthCounter();
            this.square2.clockLengthCounter();
            this.noise.clockLengthCounter();
            this.square1.clockSweep();
            this.square2.clockSweep();

        }

        if (this.derivedFrameCounter >= 0 && this.derivedFrameCounter < 4) {

            // Clock linear & decay:
            this.square1.clockEnvDecay();
            this.square2.clockEnvDecay();
            this.noise.clockEnvDecay();
            this.triangle.clockLinearCounter();

        }

        if (this.derivedFrameCounter === 3 && this.countSequence===0) {

            // Enable IRQ:
            this.frameIrqActive = true;

        }


        // End of 240Hz tick

    },


    // Samples the channels, mixes the output together,
    // writes to buffer and (if enabled) file.
    sample: function(){
        var sq_index, tnd_index;

        if (this.accCount > 0) {

            this.smpSquare1 <<= 4;
            this.smpSquare1 = Math.floor(this.smpSquare1 / this.accCount);

            this.smpSquare2 <<= 4;
            this.smpSquare2 = Math.floor(this.smpSquare2 / this.accCount);

            this.smpTriangle = Math.floor(this.smpTriangle / this.accCount);

            this.smpDmc <<= 4;
            this.smpDmc = Math.floor(this.smpDmc / this.accCount);

            this.accCount = 0;
        }
        else {
            this.smpSquare1 = this.square1.sampleValue << 4;
            this.smpSquare2 = this.square2.sampleValue << 4;
            this.smpTriangle = this.triangle.sampleValue;
            this.smpDmc = this.dmc.sample << 4;
        }

        var smpNoise = Math.floor((this.noise.accValue << 4) /
                this.noise.accCount);
        this.noise.accValue = smpNoise >> 4;
        this.noise.accCount = 1;

        // Stereo sound.

        // Left channel:
        sq_index  = (
                this.smpSquare1 * this.stereoPosLSquare1 +
                this.smpSquare2 * this.stereoPosLSquare2
            ) >> 8;
        tnd_index = (
                3 * this.smpTriangle * this.stereoPosLTriangle +
                (smpNoise<<1) * this.stereoPosLNoise + this.smpDmc *
                this.stereoPosLDMC
            ) >> 8;
        if (sq_index >= this.square_table.length) {
            sq_index  = this.square_table.length-1;
        }
        if (tnd_index >= this.tnd_table.length) {
            tnd_index = this.tnd_table.length - 1;
        }
        var sampleValueL = this.square_table[sq_index] +
                this.tnd_table[tnd_index] - this.dcValue;

        // Right channel:
        sq_index = (this.smpSquare1 * this.stereoPosRSquare1 +
                this.smpSquare2 * this.stereoPosRSquare2
            ) >> 8;
        tnd_index = (3 * this.smpTriangle * this.stereoPosRTriangle +
                (smpNoise << 1) * this.stereoPosRNoise + this.smpDmc *
                this.stereoPosRDMC
            ) >> 8;
        if (sq_index >= this.square_table.length) {
            sq_index = this.square_table.length - 1;
        }
        if (tnd_index >= this.tnd_table.length) {
            tnd_index = this.tnd_table.length - 1;
        }
        var sampleValueR = this.square_table[sq_index] +
                this.tnd_table[tnd_index] - this.dcValue;

        // Remove DC from left channel:
        var smpDiffL = sampleValueL - this.prevSampleL;
        this.prevSampleL += smpDiffL;
        this.smpAccumL += smpDiffL - (this.smpAccumL >> 10);
        sampleValueL = this.smpAccumL;

        // Remove DC from right channel:
        var smpDiffR     = sampleValueR - this.prevSampleR;
        this.prevSampleR += smpDiffR;
        this.smpAccumR  += smpDiffR - (this.smpAccumR >> 10);
        sampleValueR = this.smpAccumR;

        // Write:
        if (sampleValueL > this.maxSample) {
            this.maxSample = sampleValueL;
        }
        if (sampleValueL < this.minSample) {
            this.minSample = sampleValueL;
        }
        this.sampleBuffer[this.bufferIndex++] = sampleValueL;
        this.sampleBuffer[this.bufferIndex++] = sampleValueR;

        // Write full buffer
        if (this.bufferIndex === this.sampleBuffer.length) {
            this.nes.ui.writeAudio(this.sampleBuffer);
            this.sampleBuffer = new Array(this.bufferSize*2);
            this.bufferIndex = 0;
        }

        // Reset sampled values:
        this.smpSquare1 = 0;
        this.smpSquare2 = 0;
        this.smpTriangle = 0;
        this.smpDmc = 0;

    },

    getLengthMax: function(value){
        return this.lengthLookup[value >> 3];
    },

    getDmcFrequency: function(value){
        if (value >= 0 && value < 0x10) {
            return this.dmcFreqLookup[value];
        }
        return 0;
    },

    getNoiseWaveLength: function(value){
        if (value >= 0 && value < 0x10) {
            return this.noiseWavelengthLookup[value];
        }
        return 0;
    },

    setPanning: function(pos){
        for (var i = 0; i < 5; i++) {
            this.panning[i] = pos[i];
        }
        this.updateStereoPos();
    },

    setMasterVolume: function(value){
        if (value < 0) {
            value = 0;
        }
        if (value > 256) {
            value = 256;
        }
        this.masterVolume = value;
        this.updateStereoPos();
    },

    updateStereoPos: function(){
        this.stereoPosLSquare1 = (this.panning[0] * this.masterVolume) >> 8;
        this.stereoPosLSquare2 = (this.panning[1] * this.masterVolume) >> 8;
        this.stereoPosLTriangle = (this.panning[2] * this.masterVolume) >> 8;
        this.stereoPosLNoise = (this.panning[3] * this.masterVolume) >> 8;
        this.stereoPosLDMC = (this.panning[4] * this.masterVolume) >> 8;

        this.stereoPosRSquare1 = this.masterVolume - this.stereoPosLSquare1;
        this.stereoPosRSquare2 = this.masterVolume - this.stereoPosLSquare2;
        this.stereoPosRTriangle = this.masterVolume - this.stereoPosLTriangle;
        this.stereoPosRNoise = this.masterVolume - this.stereoPosLNoise;
        this.stereoPosRDMC = this.masterVolume - this.stereoPosLDMC;
    },

    initLengthLookup: function(){

        this.lengthLookup = [
            0x0A, 0xFE,
            0x14, 0x02,
            0x28, 0x04,
            0x50, 0x06,
            0xA0, 0x08,
            0x3C, 0x0A,
            0x0E, 0x0C,
            0x1A, 0x0E,
            0x0C, 0x10,
            0x18, 0x12,
            0x30, 0x14,
            0x60, 0x16,
            0xC0, 0x18,
            0x48, 0x1A,
            0x10, 0x1C,
            0x20, 0x1E
        ];
    },

    initDmcFrequencyLookup: function(){

        this.dmcFreqLookup = new Array(16);

        this.dmcFreqLookup[0x0] = 0xD60;
        this.dmcFreqLookup[0x1] = 0xBE0;
        this.dmcFreqLookup[0x2] = 0xAA0;
        this.dmcFreqLookup[0x3] = 0xA00;
        this.dmcFreqLookup[0x4] = 0x8F0;
        this.dmcFreqLookup[0x5] = 0x7F0;
        this.dmcFreqLookup[0x6] = 0x710;
        this.dmcFreqLookup[0x7] = 0x6B0;
        this.dmcFreqLookup[0x8] = 0x5F0;
        this.dmcFreqLookup[0x9] = 0x500;
        this.dmcFreqLookup[0xA] = 0x470;
        this.dmcFreqLookup[0xB] = 0x400;
        this.dmcFreqLookup[0xC] = 0x350;
        this.dmcFreqLookup[0xD] = 0x2A0;
        this.dmcFreqLookup[0xE] = 0x240;
        this.dmcFreqLookup[0xF] = 0x1B0;
        //for(int i=0;i<16;i++)dmcFreqLookup[i]/=8;

    },

    initNoiseWavelengthLookup: function(){

        this.noiseWavelengthLookup = new Array(16);

        this.noiseWavelengthLookup[0x0] = 0x004;
        this.noiseWavelengthLookup[0x1] = 0x008;
        this.noiseWavelengthLookup[0x2] = 0x010;
        this.noiseWavelengthLookup[0x3] = 0x020;
        this.noiseWavelengthLookup[0x4] = 0x040;
        this.noiseWavelengthLookup[0x5] = 0x060;
        this.noiseWavelengthLookup[0x6] = 0x080;
        this.noiseWavelengthLookup[0x7] = 0x0A0;
        this.noiseWavelengthLookup[0x8] = 0x0CA;
        this.noiseWavelengthLookup[0x9] = 0x0FE;
        this.noiseWavelengthLookup[0xA] = 0x17C;
        this.noiseWavelengthLookup[0xB] = 0x1FC;
        this.noiseWavelengthLookup[0xC] = 0x2FA;
        this.noiseWavelengthLookup[0xD] = 0x3F8;
        this.noiseWavelengthLookup[0xE] = 0x7F2;
        this.noiseWavelengthLookup[0xF] = 0xFE4;

    },

    initDACtables: function(){
        var value, ival, i;
        var max_sqr = 0;
        var max_tnd = 0;

        this.square_table = new Array(32*16);
        this.tnd_table = new Array(204*16);

        for (i = 0; i < 32 * 16; i++) {
            value = 95.52 / (8128.0 / (i/16.0) + 100.0);
            value *= 0.98411;
            value *= 50000.0;
            ival = Math.floor(value);

            this.square_table[i] = ival;
            if (ival > max_sqr) {
                max_sqr = ival;
            }
        }

        for (i = 0; i < 204 * 16; i++) {
            value = 163.67 / (24329.0 / (i/16.0) + 100.0);
            value *= 0.98411;
            value *= 50000.0;
            ival = Math.floor(value);

            this.tnd_table[i] = ival;
            if (ival > max_tnd) {
                max_tnd = ival;
            }

        }

        this.dacRange = max_sqr+max_tnd;
        this.dcValue = this.dacRange/2;

    }
};


JSNES.PAPU.ChannelDM = function(papu) {
    this.papu = papu;

    this.MODE_NORMAL = 0;
    this.MODE_LOOP = 1;
    this.MODE_IRQ = 2;

    this.isEnabled = null;
    this.hasSample = null;
    this.irqGenerated = false;

    this.playMode = null;
    this.dmaFrequency = null;
    this.dmaCounter = null;
    this.deltaCounter = null;
    this.playStartAddress = null;
    this.playAddress = null;
    this.playLength = null;
    this.playLengthCounter = null;
    this.shiftCounter = null;
    this.reg4012 = null;
    this.reg4013 = null;
    this.sample = null;
    this.dacLsb = null;
    this.data = null;

    this.reset();
};

JSNES.PAPU.ChannelDM.prototype = {
    clockDmc: function() {

        // Only alter DAC value if the sample buffer has data:
        if(this.hasSample) {

            if ((this.data & 1) === 0) {

                // Decrement delta:
                if(this.deltaCounter>0) {
                    this.deltaCounter--;
                }
            }
            else {
                // Increment delta:
                if (this.deltaCounter < 63) {
                    this.deltaCounter++;
                }
            }

            // Update sample value:
            this.sample = this.isEnabled ? (this.deltaCounter << 1) + this.dacLsb : 0;

            // Update shift register:
            this.data >>= 1;

        }

        this.dmaCounter--;
        if (this.dmaCounter <= 0) {

            // No more sample bits.
            this.hasSample = false;
            this.endOfSample();
            this.dmaCounter = 8;

        }

        if (this.irqGenerated) {
            this.papu.nes.cpu.requestIrq(this.papu.nes.cpu.IRQ_NORMAL);
        }

    },

    endOfSample: function() {
        if (this.playLengthCounter === 0 && this.playMode === this.MODE_LOOP) {

            // Start from beginning of sample:
            this.playAddress = this.playStartAddress;
            this.playLengthCounter = this.playLength;

        }

        if (this.playLengthCounter > 0) {

            // Fetch next sample:
            this.nextSample();

            if (this.playLengthCounter === 0) {

                // Last byte of sample fetched, generate IRQ:
                if (this.playMode === this.MODE_IRQ) {

                    // Generate IRQ:
                    this.irqGenerated = true;

                }

            }

        }

    },

    nextSample: function() {
        // Fetch byte:
        this.data = this.papu.nes.mmap.load(this.playAddress);
        this.papu.nes.cpu.haltCycles(4);

        this.playLengthCounter--;
        this.playAddress++;
        if (this.playAddress > 0xFFFF) {
            this.playAddress = 0x8000;
        }

        this.hasSample = true;
    },

    writeReg: function(address, value) {
        if (address === 0x4010) {

            // Play mode, DMA Frequency
            if ((value >> 6) === 0) {
                this.playMode = this.MODE_NORMAL;
            }
            else if (((value >> 6) & 1) === 1) {
                this.playMode = this.MODE_LOOP;
            }
            else if ((value >> 6) === 2) {
                this.playMode = this.MODE_IRQ;
            }

            if ((value & 0x80) === 0) {
                this.irqGenerated = false;
            }

            this.dmaFrequency = this.papu.getDmcFrequency(value & 0xF);

        }
        else if (address === 0x4011) {

            // Delta counter load register:
            this.deltaCounter = (value >> 1) & 63;
            this.dacLsb = value & 1;
            this.sample = ((this.deltaCounter << 1) + this.dacLsb); // update sample value

        }
        else if (address === 0x4012) {

            // DMA address load register
            this.playStartAddress = (value << 6) | 0x0C000;
            this.playAddress = this.playStartAddress;
            this.reg4012 = value;

        }
        else if (address === 0x4013) {

            // Length of play code
            this.playLength = (value << 4) + 1;
            this.playLengthCounter = this.playLength;
            this.reg4013 = value;

        }
        else if (address === 0x4015) {

            // DMC/IRQ Status
            if (((value >> 4) & 1) === 0) {
                // Disable:
                this.playLengthCounter = 0;
            }
            else {
                // Restart:
                this.playAddress = this.playStartAddress;
                this.playLengthCounter = this.playLength;
            }
            this.irqGenerated = false;
        }
    },

    setEnabled: function(value) {
        if ((!this.isEnabled) && value) {
            this.playLengthCounter = this.playLength;
        }
        this.isEnabled = value;
    },

    getLengthStatus: function(){
        return ((this.playLengthCounter === 0 || !this.isEnabled) ? 0 : 1);
    },

    getIrqStatus: function(){
        return (this.irqGenerated ? 1 : 0);
    },

    reset: function(){
        this.isEnabled = false;
        this.irqGenerated = false;
        this.playMode = this.MODE_NORMAL;
        this.dmaFrequency = 0;
        this.dmaCounter = 0;
        this.deltaCounter = 0;
        this.playStartAddress = 0;
        this.playAddress = 0;
        this.playLength = 0;
        this.playLengthCounter = 0;
        this.sample = 0;
        this.dacLsb = 0;
        this.shiftCounter = 0;
        this.reg4012 = 0;
        this.reg4013 = 0;
        this.data = 0;
    }
};


JSNES.PAPU.ChannelNoise = function(papu) {
    this.papu = papu;

    this.isEnabled = null;
    this.envDecayDisable = null;
    this.envDecayLoopEnable = null;
    this.lengthCounterEnable = null;
    this.envReset = null;
    this.shiftNow = null;

    this.lengthCounter = null;
    this.progTimerCount = null;
    this.progTimerMax = null;
    this.envDecayRate = null;
    this.envDecayCounter = null;
    this.envVolume = null;
    this.masterVolume = null;
    this.shiftReg = 1<<14;
    this.randomBit = null;
    this.randomMode = null;
    this.sampleValue = null;
    this.accValue=0;
    this.accCount=1;
    this.tmp = null;

    this.reset();
};

JSNES.PAPU.ChannelNoise.prototype = {
    reset: function() {
        this.progTimerCount = 0;
        this.progTimerMax = 0;
        this.isEnabled = false;
        this.lengthCounter = 0;
        this.lengthCounterEnable = false;
        this.envDecayDisable = false;
        this.envDecayLoopEnable = false;
        this.shiftNow = false;
        this.envDecayRate = 0;
        this.envDecayCounter = 0;
        this.envVolume = 0;
        this.masterVolume = 0;
        this.shiftReg = 1;
        this.randomBit = 0;
        this.randomMode = 0;
        this.sampleValue = 0;
        this.tmp = 0;
    },

    clockLengthCounter: function(){
        if (this.lengthCounterEnable && this.lengthCounter>0){
            this.lengthCounter--;
            if (this.lengthCounter === 0) {
                this.updateSampleValue();
            }
        }
    },

    clockEnvDecay: function() {
        if(this.envReset) {
            // Reset envelope:
            this.envReset = false;
            this.envDecayCounter = this.envDecayRate + 1;
            this.envVolume = 0xF;
        }
        else if (--this.envDecayCounter <= 0) {
            // Normal handling:
            this.envDecayCounter = this.envDecayRate + 1;
            if(this.envVolume>0) {
                this.envVolume--;
            }
            else {
                this.envVolume = this.envDecayLoopEnable ? 0xF : 0;
            }
        }
        this.masterVolume = this.envDecayDisable ? this.envDecayRate : this.envVolume;
        this.updateSampleValue();
    },

    updateSampleValue: function() {
        if (this.isEnabled && this.lengthCounter>0) {
            this.sampleValue = this.randomBit * this.masterVolume;
        }
    },

    writeReg: function(address, value){
        if(address === 0x400C) {
            // Volume/Envelope decay:
            this.envDecayDisable = ((value&0x10) !== 0);
            this.envDecayRate = value&0xF;
            this.envDecayLoopEnable = ((value&0x20) !== 0);
            this.lengthCounterEnable = ((value&0x20)===0);
            this.masterVolume = this.envDecayDisable?this.envDecayRate:this.envVolume;

        }else if(address === 0x400E) {
            // Programmable timer:
            this.progTimerMax = this.papu.getNoiseWaveLength(value&0xF);
            this.randomMode = value>>7;

        }else if(address === 0x400F) {
            // Length counter
            this.lengthCounter = this.papu.getLengthMax(value&248);
            this.envReset = true;
        }
        // Update:
        //updateSampleValue();
    },

    setEnabled: function(value){
        this.isEnabled = value;
        if (!value) {
            this.lengthCounter = 0;
        }
        this.updateSampleValue();
    },

    getLengthStatus: function() {
        return ((this.lengthCounter===0 || !this.isEnabled)?0:1);
    }
};


JSNES.PAPU.ChannelSquare = function(papu, square1) {
    this.papu = papu;

    this.dutyLookup = [
         0, 1, 0, 0, 0, 0, 0, 0,
         0, 1, 1, 0, 0, 0, 0, 0,
         0, 1, 1, 1, 1, 0, 0, 0,
         1, 0, 0, 1, 1, 1, 1, 1
    ];
    this.impLookup = [
         1,-1, 0, 0, 0, 0, 0, 0,
         1, 0,-1, 0, 0, 0, 0, 0,
         1, 0, 0, 0,-1, 0, 0, 0,
        -1, 0, 1, 0, 0, 0, 0, 0
    ];

    this.sqr1 = square1;
    this.isEnabled = null;
    this.lengthCounterEnable = null;
    this.sweepActive = null;
    this.envDecayDisable = null;
    this.envDecayLoopEnable = null;
    this.envReset = null;
    this.sweepCarry = null;
    this.updateSweepPeriod = null;

    this.progTimerCount = null;
    this.progTimerMax = null;
    this.lengthCounter = null;
    this.squareCounter = null;
    this.sweepCounter = null;
    this.sweepCounterMax = null;
    this.sweepMode = null;
    this.sweepShiftAmount = null;
    this.envDecayRate = null;
    this.envDecayCounter = null;
    this.envVolume = null;
    this.masterVolume = null;
    this.dutyMode = null;
    this.sweepResult = null;
    this.sampleValue = null;
    this.vol = null;

    this.reset();
};

JSNES.PAPU.ChannelSquare.prototype = {
    reset: function() {
        this.progTimerCount = 0;
        this.progTimerMax = 0;
        this.lengthCounter = 0;
        this.squareCounter = 0;
        this.sweepCounter = 0;
        this.sweepCounterMax = 0;
        this.sweepMode = 0;
        this.sweepShiftAmount = 0;
        this.envDecayRate = 0;
        this.envDecayCounter = 0;
        this.envVolume = 0;
        this.masterVolume = 0;
        this.dutyMode = 0;
        this.vol = 0;

        this.isEnabled = false;
        this.lengthCounterEnable = false;
        this.sweepActive = false;
        this.sweepCarry = false;
        this.envDecayDisable = false;
        this.envDecayLoopEnable = false;
    },

    clockLengthCounter: function() {
        if (this.lengthCounterEnable && this.lengthCounter > 0){
            this.lengthCounter--;
            if (this.lengthCounter === 0) {
                this.updateSampleValue();
            }
        }
    },

    clockEnvDecay: function() {
        if (this.envReset) {
            // Reset envelope:
            this.envReset = false;
            this.envDecayCounter = this.envDecayRate + 1;
            this.envVolume = 0xF;
        }else if ((--this.envDecayCounter) <= 0) {
            // Normal handling:
            this.envDecayCounter = this.envDecayRate + 1;
            if (this.envVolume>0) {
                this.envVolume--;
            }else{
                this.envVolume = this.envDecayLoopEnable ? 0xF : 0;
            }
        }

        this.masterVolume = this.envDecayDisable ? this.envDecayRate : this.envVolume;
        this.updateSampleValue();
    },

    clockSweep: function() {
        if (--this.sweepCounter<=0) {

            this.sweepCounter = this.sweepCounterMax + 1;
            if (this.sweepActive && this.sweepShiftAmount>0 && this.progTimerMax>7) {

                // Calculate result from shifter:
                this.sweepCarry = false;
                if (this.sweepMode===0) {
                    this.progTimerMax += (this.progTimerMax>>this.sweepShiftAmount);
                    if (this.progTimerMax > 4095) {
                        this.progTimerMax = 4095;
                        this.sweepCarry = true;
                    }
                }else{
                    this.progTimerMax = this.progTimerMax - ((this.progTimerMax>>this.sweepShiftAmount)-(this.sqr1?1:0));
                }
            }
        }

        if (this.updateSweepPeriod) {
            this.updateSweepPeriod = false;
            this.sweepCounter = this.sweepCounterMax + 1;
        }
    },

    updateSampleValue: function() {
        if (this.isEnabled && this.lengthCounter>0 && this.progTimerMax>7) {

            if (this.sweepMode===0 && (this.progTimerMax + (this.progTimerMax>>this.sweepShiftAmount)) > 4095) {
            //if (this.sweepCarry) {
                this.sampleValue = 0;
            }else{
                this.sampleValue = this.masterVolume*this.dutyLookup[(this.dutyMode<<3)+this.squareCounter];
            }
        }else{
            this.sampleValue = 0;
        }
    },

    writeReg: function(address, value){
        var addrAdd = (this.sqr1?0:4);
        if (address === 0x4000 + addrAdd) {
            // Volume/Envelope decay:
            this.envDecayDisable = ((value&0x10) !== 0);
            this.envDecayRate = value & 0xF;
            this.envDecayLoopEnable = ((value&0x20) !== 0);
            this.dutyMode = (value>>6)&0x3;
            this.lengthCounterEnable = ((value&0x20)===0);
            this.masterVolume = this.envDecayDisable?this.envDecayRate:this.envVolume;
            this.updateSampleValue();

        }
        else if (address === 0x4001+addrAdd) {
            // Sweep:
            this.sweepActive = ((value&0x80) !== 0);
            this.sweepCounterMax = ((value>>4)&7);
            this.sweepMode = (value>>3)&1;
            this.sweepShiftAmount = value&7;
            this.updateSweepPeriod = true;
        }
        else if (address === 0x4002+addrAdd){
            // Programmable timer:
            this.progTimerMax &= 0x700;
            this.progTimerMax |= value;
        }
        else if (address === 0x4003+addrAdd) {
            // Programmable timer, length counter
            this.progTimerMax &= 0xFF;
            this.progTimerMax |= ((value&0x7)<<8);

            if (this.isEnabled){
                this.lengthCounter = this.papu.getLengthMax(value&0xF8);
            }

            this.envReset  = true;
        }
    },

    setEnabled: function(value) {
        this.isEnabled = value;
        if (!value) {
            this.lengthCounter = 0;
        }
        this.updateSampleValue();
    },

    getLengthStatus: function() {
        return ((this.lengthCounter === 0 || !this.isEnabled) ? 0 : 1);
    }
};


JSNES.PAPU.ChannelTriangle = function(papu) {
    this.papu = papu;

    this.isEnabled = null;
    this.sampleCondition = null;
    this.lengthCounterEnable = null;
    this.lcHalt = null;
    this.lcControl = null;

    this.progTimerCount = null;
    this.progTimerMax = null;
    this.triangleCounter = null;
    this.lengthCounter = null;
    this.linearCounter = null;
    this.lcLoadValue = null;
    this.sampleValue = null;
    this.tmp = null;

    this.reset();
};

JSNES.PAPU.ChannelTriangle.prototype = {
    reset: function(){
        this.progTimerCount = 0;
        this.progTimerMax = 0;
        this.triangleCounter = 0;
        this.isEnabled = false;
        this.sampleCondition = false;
        this.lengthCounter = 0;
        this.lengthCounterEnable = false;
        this.linearCounter = 0;
        this.lcLoadValue = 0;
        this.lcHalt = true;
        this.lcControl = false;
        this.tmp = 0;
        this.sampleValue = 0xF;
    },

    clockLengthCounter: function(){
        if (this.lengthCounterEnable && this.lengthCounter>0) {
            this.lengthCounter--;
            if (this.lengthCounter===0) {
                this.updateSampleCondition();
            }
        }
    },

    clockLinearCounter: function(){
        if (this.lcHalt){
            // Load:
            this.linearCounter = this.lcLoadValue;
            this.updateSampleCondition();
        }
        else if (this.linearCounter > 0) {
            // Decrement:
            this.linearCounter--;
            this.updateSampleCondition();
        }
        if (!this.lcControl) {
            // Clear halt flag:
            this.lcHalt = false;
        }
    },

    getLengthStatus: function(){
        return ((this.lengthCounter === 0 || !this.isEnabled)?0:1);
    },

    readReg: function(address){
        return 0;
    },

    writeReg: function(address, value){
        if (address === 0x4008) {
            // New values for linear counter:
            this.lcControl  = (value&0x80)!==0;
            this.lcLoadValue =  value&0x7F;

            // Length counter enable:
            this.lengthCounterEnable = !this.lcControl;
        }
        else if (address === 0x400A) {
            // Programmable timer:
            this.progTimerMax &= 0x700;
            this.progTimerMax |= value;

        }
        else if(address === 0x400B) {
            // Programmable timer, length counter
            this.progTimerMax &= 0xFF;
            this.progTimerMax |= ((value&0x07)<<8);
            this.lengthCounter = this.papu.getLengthMax(value&0xF8);
            this.lcHalt = true;
        }

        this.updateSampleCondition();
    },

    clockProgrammableTimer: function(nCycles){
        if (this.progTimerMax>0) {
            this.progTimerCount += nCycles;
            while (this.progTimerMax > 0 &&
                    this.progTimerCount >= this.progTimerMax) {
                this.progTimerCount -= this.progTimerMax;
                if (this.isEnabled && this.lengthCounter>0 &&
                        this.linearCounter > 0) {
                    this.clockTriangleGenerator();
                }
            }
        }
    },

    clockTriangleGenerator: function() {
        this.triangleCounter++;
        this.triangleCounter &= 0x1F;
    },

    setEnabled: function(value) {
        this.isEnabled = value;
        if(!value) {
            this.lengthCounter = 0;
        }
        this.updateSampleCondition();
    },

    updateSampleCondition: function() {
        this.sampleCondition = this.isEnabled &&
                this.progTimerMax > 7 &&
                this.linearCounter > 0 &&
                this.lengthCounter > 0;
    }
};

/*
JSNES, based on Jamie Sanders' vNES
Copyright (C) 2010 Ben Firshman

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

JSNES.PPU = function(nes) {
    this.nes = nes;

    // Keep Chrome happy
    this.vramMem = null;
    this.spriteMem = null;
    this.vramAddress = null;
    this.vramTmpAddress = null;
    this.vramBufferedReadValue = null;
    this.firstWrite = null;
    this.sramAddress = null;
    this.currentMirroring = null;
    this.requestEndFrame = null;
    this.nmiOk = null;
    this.dummyCycleToggle = null;
    this.validTileData = null;
    this.nmiCounter = null;
    this.scanlineAlreadyRendered = null;
    this.f_nmiOnVblank = null;
    this.f_spriteSize = null;
    this.f_bgPatternTable = null;
    this.f_spPatternTable = null;
    this.f_addrInc = null;
    this.f_nTblAddress = null;
    this.f_color = null;
    this.f_spVisibility = null;
    this.f_bgVisibility = null;
    this.f_spClipping = null;
    this.f_bgClipping = null;
    this.f_dispType = null;
    this.cntFV = null;
    this.cntV = null;
    this.cntH = null;
    this.cntVT = null;
    this.cntHT = null;
    this.regFV = null;
    this.regV = null;
    this.regH = null;
    this.regVT = null;
    this.regHT = null;
    this.regFH = null;
    this.regS = null;
    this.curNt = null;
    this.attrib = null;
    this.buffer = null;
    this.prevBuffer = null;
    this.bgbuffer = null;
    this.pixrendered = null;

    this.validTileData = null;
    this.scantile = null;
    this.scanline = null;
    this.lastRenderedScanline = null;
    this.curX = null;
    this.sprX = null;
    this.sprY = null;
    this.sprTile = null;
    this.sprCol = null;
    this.vertFlip = null;
    this.horiFlip = null;
    this.bgPriority = null;
    this.spr0HitX = null;
    this.spr0HitY = null;
    this.hitSpr0 = null;
    this.sprPalette = null;
    this.imgPalette = null;
    this.ptTile = null;
    this.ntable1 = null;
    this.currentMirroring = null;
    this.nameTable = null;
    this.vramMirrorTable = null;
    this.palTable = null;


    // Rendering Options:
    this.showSpr0Hit = false;
    this.clipToTvSize = true;

    this.reset();
};

JSNES.PPU.prototype = {
    // Status flags:
    STATUS_VRAMWRITE: 4,
    STATUS_SLSPRITECOUNT: 5,
    STATUS_SPRITE0HIT: 6,
    STATUS_VBLANK: 7,

    reset: function() {
        var i;

        // Memory
        this.vramMem = new Array(0x8000);
        this.spriteMem = new Array(0x100);
        for (i=0; i<this.vramMem.length; i++) {
            this.vramMem[i] = 0;
        }
        for (i=0; i<this.spriteMem.length; i++) {
            this.spriteMem[i] = 0;
        }

        // VRAM I/O:
        this.vramAddress = null;
        this.vramTmpAddress = null;
        this.vramBufferedReadValue = 0;
        this.firstWrite = true;       // VRAM/Scroll Hi/Lo latch

        // SPR-RAM I/O:
        this.sramAddress = 0; // 8-bit only.

        this.currentMirroring = -1;
        this.requestEndFrame = false;
        this.nmiOk = false;
        this.dummyCycleToggle = false;
        this.validTileData = false;
        this.nmiCounter = 0;
        this.scanlineAlreadyRendered = null;

        // Control Flags Register 1:
        this.f_nmiOnVblank = 0;    // NMI on VBlank. 0=disable, 1=enable
        this.f_spriteSize = 0;     // Sprite size. 0=8x8, 1=8x16
        this.f_bgPatternTable = 0; // Background Pattern Table address. 0=0x0000,1=0x1000
        this.f_spPatternTable = 0; // Sprite Pattern Table address. 0=0x0000,1=0x1000
        this.f_addrInc = 0;        // PPU Address Increment. 0=1,1=32
        this.f_nTblAddress = 0;    // Name Table Address. 0=0x2000,1=0x2400,2=0x2800,3=0x2C00

        // Control Flags Register 2:
        this.f_color = 0;         // Background color. 0=black, 1=blue, 2=green, 4=red
        this.f_spVisibility = 0;   // Sprite visibility. 0=not displayed,1=displayed
        this.f_bgVisibility = 0;   // Background visibility. 0=Not Displayed,1=displayed
        this.f_spClipping = 0;     // Sprite clipping. 0=Sprites invisible in left 8-pixel column,1=No clipping
        this.f_bgClipping = 0;     // Background clipping. 0=BG invisible in left 8-pixel column, 1=No clipping
        this.f_dispType = 0;       // Display type. 0=color, 1=monochrome

        // Counters:
        this.cntFV = 0;
        this.cntV = 0;
        this.cntH = 0;
        this.cntVT = 0;
        this.cntHT = 0;

        // Registers:
        this.regFV = 0;
        this.regV = 0;
        this.regH = 0;
        this.regVT = 0;
        this.regHT = 0;
        this.regFH = 0;
        this.regS = 0;

        // These are temporary variables used in rendering and sound procedures.
        // Their states outside of those procedures can be ignored.
        // TODO: the use of this is a bit weird, investigate
        this.curNt = null;

        // Variables used when rendering:
        this.attrib = new Array(32);
        this.buffer = new Array(256*240);
        this.prevBuffer = new Array(256*240);
        this.bgbuffer = new Array(256*240);
        this.pixrendered = new Array(256*240);

        this.validTileData = null;

        this.scantile = new Array(32);

        // Initialize misc vars:
        this.scanline = 0;
        this.lastRenderedScanline = -1;
        this.curX = 0;

        // Sprite data:
        this.sprX = new Array(64); // X coordinate
        this.sprY = new Array(64); // Y coordinate
        this.sprTile = new Array(64); // Tile Index (into pattern table)
        this.sprCol = new Array(64); // Upper two bits of color
        this.vertFlip = new Array(64); // Vertical Flip
        this.horiFlip = new Array(64); // Horizontal Flip
        this.bgPriority = new Array(64); // Background priority
        this.spr0HitX = 0; // Sprite #0 hit X coordinate
        this.spr0HitY = 0; // Sprite #0 hit Y coordinate
        this.hitSpr0 = false;

        // Palette data:
        this.sprPalette = new Array(16);
        this.imgPalette = new Array(16);

        // Create pattern table tile buffers:
        this.ptTile = new Array(512);
        for (i=0; i<512; i++) {
            this.ptTile[i] = new JSNES.PPU.Tile();
        }

        // Create nametable buffers:
        // Name table data:
        this.ntable1 = new Array(4);
        this.currentMirroring = -1;
        this.nameTable = new Array(4);
        for (i=0; i<4; i++) {
            this.nameTable[i] = new JSNES.PPU.NameTable(32, 32, "Nt"+i);
        }

        // Initialize mirroring lookup table:
        this.vramMirrorTable = new Array(0x8000);
        for (i=0; i<0x8000; i++) {
            this.vramMirrorTable[i] = i;
        }

        this.palTable = new JSNES.PPU.PaletteTable();
        this.palTable.loadNTSCPalette();
        //this.palTable.loadDefaultPalette();

        this.updateControlReg1(0);
        this.updateControlReg2(0);
    },

    // Sets Nametable mirroring.
    setMirroring: function(mirroring){

        if (mirroring == this.currentMirroring) {
            return;
        }

        this.currentMirroring = mirroring;
        this.triggerRendering();

        // Remove mirroring:
        if (this.vramMirrorTable === null) {
            this.vramMirrorTable = new Array(0x8000);
        }
        for (var i=0; i<0x8000; i++) {
            this.vramMirrorTable[i] = i;
        }

        // Palette mirroring:
        this.defineMirrorRegion(0x3f20,0x3f00,0x20);
        this.defineMirrorRegion(0x3f40,0x3f00,0x20);
        this.defineMirrorRegion(0x3f80,0x3f00,0x20);
        this.defineMirrorRegion(0x3fc0,0x3f00,0x20);

        // Additional mirroring:
        this.defineMirrorRegion(0x3000,0x2000,0xf00);
        this.defineMirrorRegion(0x4000,0x0000,0x4000);

        if (mirroring == this.nes.rom.HORIZONTAL_MIRRORING) {
            // Horizontal mirroring.

            this.ntable1[0] = 0;
            this.ntable1[1] = 0;
            this.ntable1[2] = 1;
            this.ntable1[3] = 1;

            this.defineMirrorRegion(0x2400,0x2000,0x400);
            this.defineMirrorRegion(0x2c00,0x2800,0x400);

        }else if (mirroring == this.nes.rom.VERTICAL_MIRRORING) {
            // Vertical mirroring.

            this.ntable1[0] = 0;
            this.ntable1[1] = 1;
            this.ntable1[2] = 0;
            this.ntable1[3] = 1;

            this.defineMirrorRegion(0x2800,0x2000,0x400);
            this.defineMirrorRegion(0x2c00,0x2400,0x400);

        }else if (mirroring == this.nes.rom.SINGLESCREEN_MIRRORING) {

            // Single Screen mirroring

            this.ntable1[0] = 0;
            this.ntable1[1] = 0;
            this.ntable1[2] = 0;
            this.ntable1[3] = 0;

            this.defineMirrorRegion(0x2400,0x2000,0x400);
            this.defineMirrorRegion(0x2800,0x2000,0x400);
            this.defineMirrorRegion(0x2c00,0x2000,0x400);

        }else if (mirroring == this.nes.rom.SINGLESCREEN_MIRRORING2) {


            this.ntable1[0] = 1;
            this.ntable1[1] = 1;
            this.ntable1[2] = 1;
            this.ntable1[3] = 1;

            this.defineMirrorRegion(0x2400,0x2400,0x400);
            this.defineMirrorRegion(0x2800,0x2400,0x400);
            this.defineMirrorRegion(0x2c00,0x2400,0x400);

        }else {

            // Assume Four-screen mirroring.

            this.ntable1[0] = 0;
            this.ntable1[1] = 1;
            this.ntable1[2] = 2;
            this.ntable1[3] = 3;

        }

    },


    // Define a mirrored area in the address lookup table.
    // Assumes the regions don't overlap.
    // The 'to' region is the region that is physically in memory.
    defineMirrorRegion: function(fromStart, toStart, size){
        for (var i=0;i<size;i++) {
            this.vramMirrorTable[fromStart+i] = toStart+i;
        }
    },

    startVBlank: function(){

        // Do NMI:
        this.nes.cpu.requestIrq(this.nes.cpu.IRQ_NMI);

        // Make sure everything is rendered:
        if (this.lastRenderedScanline < 239) {
            this.renderFramePartially(
                this.lastRenderedScanline+1,240-this.lastRenderedScanline
            );
        }

        // End frame:
        this.endFrame();

        // Reset scanline counter:
        this.lastRenderedScanline = -1;
    },

    endScanline: function(){
        switch (this.scanline) {
            case 19:
                // Dummy scanline.
                // May be variable length:
                if (this.dummyCycleToggle) {

                    // Remove dead cycle at end of scanline,
                    // for next scanline:
                    this.curX = 1;
                    this.dummyCycleToggle = !this.dummyCycleToggle;

                }
                break;

            case 20:
                // Clear VBlank flag:
                this.setStatusFlag(this.STATUS_VBLANK,false);

                // Clear Sprite #0 hit flag:
                this.setStatusFlag(this.STATUS_SPRITE0HIT,false);
                this.hitSpr0 = false;
                this.spr0HitX = -1;
                this.spr0HitY = -1;

                if (this.f_bgVisibility == 1 || this.f_spVisibility==1) {

                    // Update counters:
                    this.cntFV = this.regFV;
                    this.cntV = this.regV;
                    this.cntH = this.regH;
                    this.cntVT = this.regVT;
                    this.cntHT = this.regHT;

                    if (this.f_bgVisibility==1) {
                        // Render dummy scanline:
                        this.renderBgScanline(false,0);
                    }

                }

                if (this.f_bgVisibility==1 && this.f_spVisibility==1) {

                    // Check sprite 0 hit for first scanline:
                    this.checkSprite0(0);

                }

                if (this.f_bgVisibility==1 || this.f_spVisibility==1) {
                    // Clock mapper IRQ Counter:
                    this.nes.mmap.clockIrqCounter();
                }
                break;

            case 261:
                // Dead scanline, no rendering.
                // Set VINT:
                this.setStatusFlag(this.STATUS_VBLANK,true);
                this.requestEndFrame = true;
                this.nmiCounter = 9;

                // Wrap around:
                this.scanline = -1; // will be incremented to 0

                break;

            default:
                if (this.scanline >= 21 && this.scanline <= 260) {

                    // Render normally:
                    if (this.f_bgVisibility == 1) {

                        if (!this.scanlineAlreadyRendered) {
                            // update scroll:
                            this.cntHT = this.regHT;
                            this.cntH = this.regH;
                            this.renderBgScanline(true,this.scanline+1-21);
                        }
                        this.scanlineAlreadyRendered=false;

                        // Check for sprite 0 (next scanline):
                        if (!this.hitSpr0 && this.f_spVisibility == 1) {
                            if (this.sprX[0] >= -7 &&
                                    this.sprX[0] < 256 &&
                                    this.sprY[0] + 1 <= (this.scanline - 20) &&
                                    (this.sprY[0] + 1 + (
                                        this.f_spriteSize === 0 ? 8 : 16
                                    )) >= (this.scanline - 20)) {
                                if (this.checkSprite0(this.scanline - 20)) {
                                    this.hitSpr0 = true;
                                }
                            }
                        }

                    }

                    if (this.f_bgVisibility==1 || this.f_spVisibility==1) {
                        // Clock mapper IRQ Counter:
                        this.nes.mmap.clockIrqCounter();
                    }
                }
        }

        this.scanline++;
        this.regsToAddress();
        this.cntsToAddress();

    },

    startFrame: function(){
        // Set background color:
        var bgColor=0;

        if (this.f_dispType === 0) {
            // Color display.
            // f_color determines color emphasis.
            // Use first entry of image palette as BG color.
            bgColor = this.imgPalette[0];
        }
        else {
            // Monochrome display.
            // f_color determines the bg color.
            switch (this.f_color) {
                case 0:
                    // Black
                    bgColor = 0x00000;
                    break;
                case 1:
                    // Green
                    bgColor = 0x00FF00;
                    break;
                case 2:
                    // Blue
                    bgColor = 0xFF0000;
                    break;
                case 3:
                    // Invalid. Use black.
                    bgColor = 0x000000;
                    break;
                case 4:
                    // Red
                    bgColor = 0x0000FF;
                    break;
                default:
                    // Invalid. Use black.
                    bgColor = 0x0;
            }
        }

        var buffer = this.buffer;
        var i;
        for (i=0; i<256*240; i++) {
            buffer[i] = bgColor;
        }
        var pixrendered = this.pixrendered;
        for (i=0; i<pixrendered.length; i++) {
            pixrendered[i]=65;
        }
    },

    endFrame: function(){
        var i, x, y;
        var buffer = this.buffer;

        // Draw spr#0 hit coordinates:
        if (this.showSpr0Hit) {
            // Spr 0 position:
            if (this.sprX[0] >= 0 && this.sprX[0] < 256 &&
                    this.sprY[0] >= 0 && this.sprY[0] < 240) {
                for (i=0; i<256; i++) {
                    buffer[(this.sprY[0]<<8)+i] = 0xFF5555;
                }
                for (i=0; i<240; i++) {
                    buffer[(i<<8)+this.sprX[0]] = 0xFF5555;
                }
            }
            // Hit position:
            if (this.spr0HitX >= 0 && this.spr0HitX < 256 &&
                    this.spr0HitY >= 0 && this.spr0HitY < 240) {
                for (i=0; i<256; i++) {
                    buffer[(this.spr0HitY<<8)+i] = 0x55FF55;
                }
                for (i=0; i<240; i++) {
                    buffer[(i<<8)+this.spr0HitX] = 0x55FF55;
                }
            }
        }

        // This is a bit lazy..
        // if either the sprites or the background should be clipped,
        // both are clipped after rendering is finished.
        if (this.clipToTvSize || this.f_bgClipping === 0 || this.f_spClipping === 0) {
            // Clip left 8-pixels column:
            for (y=0;y<240;y++) {
                for (x=0;x<8;x++) {
                    buffer[(y<<8)+x] = 0;
                }
            }
        }

        if (this.clipToTvSize) {
            // Clip right 8-pixels column too:
            for (y=0; y<240; y++) {
                for (x=0; x<8; x++) {
                    buffer[(y<<8)+255-x] = 0;
                }
            }
        }

        // Clip top and bottom 8 pixels:
        if (this.clipToTvSize) {
            for (y=0; y<8; y++) {
                for (x=0; x<256; x++) {
                    buffer[(y<<8)+x] = 0;
                    buffer[((239-y)<<8)+x] = 0;
                }
            }
        }

        if (this.nes.opts.showDisplay) {
            this.nes.ui.writeFrame(buffer, this.prevBuffer);
        }
    },

    updateControlReg1: function(value){

        this.triggerRendering();

        this.f_nmiOnVblank =    (value>>7)&1;
        this.f_spriteSize =     (value>>5)&1;
        this.f_bgPatternTable = (value>>4)&1;
        this.f_spPatternTable = (value>>3)&1;
        this.f_addrInc =        (value>>2)&1;
        this.f_nTblAddress =     value&3;

        this.regV = (value>>1)&1;
        this.regH = value&1;
        this.regS = (value>>4)&1;

    },

    updateControlReg2: function(value){

        this.triggerRendering();

        this.f_color =       (value>>5)&7;
        this.f_spVisibility = (value>>4)&1;
        this.f_bgVisibility = (value>>3)&1;
        this.f_spClipping =   (value>>2)&1;
        this.f_bgClipping =   (value>>1)&1;
        this.f_dispType =      value&1;

        if (this.f_dispType === 0) {
            this.palTable.setEmphasis(this.f_color);
        }
        this.updatePalettes();
    },

    setStatusFlag: function(flag, value){
        var n = 1<<flag;
        this.nes.cpu.mem[0x2002] =
            ((this.nes.cpu.mem[0x2002] & (255-n)) | (value?n:0));
    },

    // CPU Register $2002:
    // Read the Status Register.
    readStatusRegister: function(){

        var tmp = this.nes.cpu.mem[0x2002];

        // Reset scroll & VRAM Address toggle:
        this.firstWrite = true;

        // Clear VBlank flag:
        this.setStatusFlag(this.STATUS_VBLANK,false);

        // Fetch status data:
        return tmp;

    },

    // CPU Register $2003:
    // Write the SPR-RAM address that is used for sramWrite (Register 0x2004 in CPU memory map)
    writeSRAMAddress: function(address) {
        this.sramAddress = address;
    },

    // CPU Register $2004 (R):
    // Read from SPR-RAM (Sprite RAM).
    // The address should be set first.
    sramLoad: function() {
        /*short tmp = sprMem.load(sramAddress);
        sramAddress++; // Increment address
        sramAddress%=0x100;
        return tmp;*/
        return this.spriteMem[this.sramAddress];
    },

    // CPU Register $2004 (W):
    // Write to SPR-RAM (Sprite RAM).
    // The address should be set first.
    sramWrite: function(value){
        this.spriteMem[this.sramAddress] = value;
        this.spriteRamWriteUpdate(this.sramAddress,value);
        this.sramAddress++; // Increment address
        this.sramAddress %= 0x100;
    },

    // CPU Register $2005:
    // Write to scroll registers.
    // The first write is the vertical offset, the second is the
    // horizontal offset:
    scrollWrite: function(value){
        this.triggerRendering();

        if (this.firstWrite) {
            // First write, horizontal scroll:
            this.regHT = (value>>3)&31;
            this.regFH = value&7;

        }else {

            // Second write, vertical scroll:
            this.regFV = value&7;
            this.regVT = (value>>3)&31;

        }
        this.firstWrite = !this.firstWrite;

    },

    // CPU Register $2006:
    // Sets the adress used when reading/writing from/to VRAM.
    // The first write sets the high byte, the second the low byte.
    writeVRAMAddress: function(address){

        if (this.firstWrite) {

            this.regFV = (address>>4)&3;
            this.regV = (address>>3)&1;
            this.regH = (address>>2)&1;
            this.regVT = (this.regVT&7) | ((address&3)<<3);

        }else {
            this.triggerRendering();

            this.regVT = (this.regVT&24) | ((address>>5)&7);
            this.regHT = address&31;

            this.cntFV = this.regFV;
            this.cntV = this.regV;
            this.cntH = this.regH;
            this.cntVT = this.regVT;
            this.cntHT = this.regHT;

            this.checkSprite0(this.scanline-20);

        }

        this.firstWrite = !this.firstWrite;

        // Invoke mapper latch:
        this.cntsToAddress();
        if (this.vramAddress < 0x2000) {
            this.nes.mmap.latchAccess(this.vramAddress);
        }
    },

    // CPU Register $2007(R):
    // Read from PPU memory. The address should be set first.
    vramLoad: function(){
        var tmp;

        this.cntsToAddress();
        this.regsToAddress();

        // If address is in range 0x0000-0x3EFF, return buffered values:
        if (this.vramAddress <= 0x3EFF) {
            tmp = this.vramBufferedReadValue;

            // Update buffered value:
            if (this.vramAddress < 0x2000) {
                this.vramBufferedReadValue = this.vramMem[this.vramAddress];
            }
            else {
                this.vramBufferedReadValue = this.mirroredLoad(
                    this.vramAddress
                );
            }

            // Mapper latch access:
            if (this.vramAddress < 0x2000) {
                this.nes.mmap.latchAccess(this.vramAddress);
            }

            // Increment by either 1 or 32, depending on d2 of Control Register 1:
            this.vramAddress += (this.f_addrInc == 1 ? 32 : 1);

            this.cntsFromAddress();
            this.regsFromAddress();

            return tmp; // Return the previous buffered value.
        }

        // No buffering in this mem range. Read normally.
        tmp = this.mirroredLoad(this.vramAddress);

        // Increment by either 1 or 32, depending on d2 of Control Register 1:
        this.vramAddress += (this.f_addrInc == 1 ? 32 : 1);

        this.cntsFromAddress();
        this.regsFromAddress();

        return tmp;
    },

    // CPU Register $2007(W):
    // Write to PPU memory. The address should be set first.
    vramWrite: function(value){

        this.triggerRendering();
        this.cntsToAddress();
        this.regsToAddress();

        if (this.vramAddress >= 0x2000) {
            // Mirroring is used.
            this.mirroredWrite(this.vramAddress,value);
        }else {

            // Write normally.
            this.writeMem(this.vramAddress,value);

            // Invoke mapper latch:
            this.nes.mmap.latchAccess(this.vramAddress);

        }

        // Increment by either 1 or 32, depending on d2 of Control Register 1:
        this.vramAddress += (this.f_addrInc==1?32:1);
        this.regsFromAddress();
        this.cntsFromAddress();

    },

    // CPU Register $4014:
    // Write 256 bytes of main memory
    // into Sprite RAM.
    sramDMA: function(value){
        var baseAddress = value * 0x100;
        var data;
        for (var i=this.sramAddress; i < 256; i++) {
            data = this.nes.cpu.mem[baseAddress+i];
            this.spriteMem[i] = data;
            this.spriteRamWriteUpdate(i, data);
        }

        this.nes.cpu.haltCycles(513);

    },

    // Updates the scroll registers from a new VRAM address.
    regsFromAddress: function(){

        var address = (this.vramTmpAddress>>8)&0xFF;
        this.regFV = (address>>4)&7;
        this.regV = (address>>3)&1;
        this.regH = (address>>2)&1;
        this.regVT = (this.regVT&7) | ((address&3)<<3);

        address = this.vramTmpAddress&0xFF;
        this.regVT = (this.regVT&24) | ((address>>5)&7);
        this.regHT = address&31;
    },

    // Updates the scroll registers from a new VRAM address.
    cntsFromAddress: function(){

        var address = (this.vramAddress>>8)&0xFF;
        this.cntFV = (address>>4)&3;
        this.cntV = (address>>3)&1;
        this.cntH = (address>>2)&1;
        this.cntVT = (this.cntVT&7) | ((address&3)<<3);

        address = this.vramAddress&0xFF;
        this.cntVT = (this.cntVT&24) | ((address>>5)&7);
        this.cntHT = address&31;

    },

    regsToAddress: function(){
        var b1  = (this.regFV&7)<<4;
        b1 |= (this.regV&1)<<3;
        b1 |= (this.regH&1)<<2;
        b1 |= (this.regVT>>3)&3;

        var b2  = (this.regVT&7)<<5;
        b2 |= this.regHT&31;

        this.vramTmpAddress = ((b1<<8) | b2)&0x7FFF;
    },

    cntsToAddress: function(){
        var b1  = (this.cntFV&7)<<4;
        b1 |= (this.cntV&1)<<3;
        b1 |= (this.cntH&1)<<2;
        b1 |= (this.cntVT>>3)&3;

        var b2  = (this.cntVT&7)<<5;
        b2 |= this.cntHT&31;

        this.vramAddress = ((b1<<8) | b2)&0x7FFF;
    },

    incTileCounter: function(count) {
        for (var i=count; i!==0; i--) {
            this.cntHT++;
            if (this.cntHT == 32) {
                this.cntHT = 0;
                this.cntVT++;
                if (this.cntVT >= 30) {
                    this.cntH++;
                    if(this.cntH == 2) {
                        this.cntH = 0;
                        this.cntV++;
                        if (this.cntV == 2) {
                            this.cntV = 0;
                            this.cntFV++;
                            this.cntFV &= 0x7;
                        }
                    }
                }
            }
        }
    },

    // Reads from memory, taking into account
    // mirroring/mapping of address ranges.
    mirroredLoad: function(address) {
        return this.vramMem[this.vramMirrorTable[address]];
    },

    // Writes to memory, taking into account
    // mirroring/mapping of address ranges.
    mirroredWrite: function(address, value){
        if (address>=0x3f00 && address<0x3f20) {
            // Palette write mirroring.
            if (address==0x3F00 || address==0x3F10) {
                this.writeMem(0x3F00,value);
                this.writeMem(0x3F10,value);

            }else if (address==0x3F04 || address==0x3F14) {

                this.writeMem(0x3F04,value);
                this.writeMem(0x3F14,value);

            }else if (address==0x3F08 || address==0x3F18) {

                this.writeMem(0x3F08,value);
                this.writeMem(0x3F18,value);

            }else if (address==0x3F0C || address==0x3F1C) {

                this.writeMem(0x3F0C,value);
                this.writeMem(0x3F1C,value);

            }else {
                this.writeMem(address,value);
            }

        }else {

            // Use lookup table for mirrored address:
            if (address<this.vramMirrorTable.length) {
                this.writeMem(this.vramMirrorTable[address],value);
            }else {
                // FIXME
                alert("Invalid VRAM address: "+address.toString(16));
            }

        }
    },

    triggerRendering: function(){
        if (this.scanline >= 21 && this.scanline <= 260) {
            // Render sprites, and combine:
            this.renderFramePartially(
                this.lastRenderedScanline+1,
                this.scanline-21-this.lastRenderedScanline
            );

            // Set last rendered scanline:
            this.lastRenderedScanline = this.scanline-21;
        }
    },

    renderFramePartially: function(startScan, scanCount){
        if (this.f_spVisibility == 1) {
            this.renderSpritesPartially(startScan,scanCount,true);
        }

        if(this.f_bgVisibility == 1) {
            var si = startScan<<8;
            var ei = (startScan+scanCount)<<8;
            if (ei > 0xF000) {
                ei = 0xF000;
            }
            var buffer = this.buffer;
            var bgbuffer = this.bgbuffer;
            var pixrendered = this.pixrendered;
            for (var destIndex=si; destIndex<ei; destIndex++) {
                if (pixrendered[destIndex] > 0xFF) {
                    buffer[destIndex] = bgbuffer[destIndex];
                }
            }
        }

        if (this.f_spVisibility == 1) {
            this.renderSpritesPartially(startScan, scanCount, false);
        }

        this.validTileData = false;
    },

    renderBgScanline: function(bgbuffer, scan) {
        var baseTile = (this.regS === 0 ? 0 : 256);
        var destIndex = (scan<<8)-this.regFH;

        this.curNt = this.ntable1[this.cntV+this.cntV+this.cntH];

        this.cntHT = this.regHT;
        this.cntH = this.regH;
        this.curNt = this.ntable1[this.cntV+this.cntV+this.cntH];

        if (scan<240 && (scan-this.cntFV)>=0){

            var tscanoffset = this.cntFV<<3;
            var scantile = this.scantile;
            var attrib = this.attrib;
            var ptTile = this.ptTile;
            var nameTable = this.nameTable;
            var imgPalette = this.imgPalette;
            var pixrendered = this.pixrendered;
            var targetBuffer = bgbuffer ? this.bgbuffer : this.buffer;

            var t, tpix, att, col;

            for (var tile=0;tile<32;tile++) {

                if (scan>=0) {

                    // Fetch tile & attrib data:
                    if (this.validTileData) {
                        // Get data from array:
                        t = scantile[tile];
                        if (typeof t === 'undefined') {
                            continue;
                        }
                        tpix = t.pix;
                        att = attrib[tile];
                    }else {
                        // Fetch data:
                        t = ptTile[baseTile+nameTable[this.curNt].getTileIndex(this.cntHT,this.cntVT)];
                        if (typeof t === 'undefined') {
                            continue;
                        }
                        tpix = t.pix;
                        att = nameTable[this.curNt].getAttrib(this.cntHT,this.cntVT);
                        scantile[tile] = t;
                        attrib[tile] = att;
                    }

                    // Render tile scanline:
                    var sx = 0;
                    var x = (tile<<3)-this.regFH;

                    if (x>-8) {
                        if (x<0) {
                            destIndex-=x;
                            sx = -x;
                        }
                        if (t.opaque[this.cntFV]) {
                            for (;sx<8;sx++) {
                                targetBuffer[destIndex] = imgPalette[
                                    tpix[tscanoffset+sx]+att
                                ];
                                pixrendered[destIndex] |= 256;
                                destIndex++;
                            }
                        }else {
                            for (;sx<8;sx++) {
                                col = tpix[tscanoffset+sx];
                                if(col !== 0) {
                                    targetBuffer[destIndex] = imgPalette[
                                        col+att
                                    ];
                                    pixrendered[destIndex] |= 256;
                                }
                                destIndex++;
                            }
                        }
                    }

                }

                // Increase Horizontal Tile Counter:
                if (++this.cntHT==32) {
                    this.cntHT=0;
                    this.cntH++;
                    this.cntH%=2;
                    this.curNt = this.ntable1[(this.cntV<<1)+this.cntH];
                }


            }

            // Tile data for one row should now have been fetched,
            // so the data in the array is valid.
            this.validTileData = true;

        }

        // update vertical scroll:
        this.cntFV++;
        if (this.cntFV==8) {
            this.cntFV = 0;
            this.cntVT++;
            if (this.cntVT==30) {
                this.cntVT = 0;
                this.cntV++;
                this.cntV%=2;
                this.curNt = this.ntable1[(this.cntV<<1)+this.cntH];
            }else if (this.cntVT==32) {
                this.cntVT = 0;
            }

            // Invalidate fetched data:
            this.validTileData = false;

        }
    },

    renderSpritesPartially: function(startscan, scancount, bgPri){
        if (this.f_spVisibility === 1) {

            for (var i=0;i<64;i++) {
                if (this.bgPriority[i]==bgPri && this.sprX[i]>=0 &&
                        this.sprX[i]<256 && this.sprY[i]+8>=startscan &&
                        this.sprY[i]<startscan+scancount) {
                    // Show sprite.
                    if (this.f_spriteSize === 0) {
                        // 8x8 sprites

                        this.srcy1 = 0;
                        this.srcy2 = 8;

                        if (this.sprY[i]<startscan) {
                            this.srcy1 = startscan - this.sprY[i]-1;
                        }

                        if (this.sprY[i]+8 > startscan+scancount) {
                            this.srcy2 = startscan+scancount-this.sprY[i]+1;
                        }

                        if (this.f_spPatternTable===0) {
                            this.ptTile[this.sprTile[i]].render(this.buffer,
                                0, this.srcy1, 8, this.srcy2, this.sprX[i],
                                this.sprY[i]+1, this.sprCol[i], this.sprPalette,
                                this.horiFlip[i], this.vertFlip[i], i,
                                this.pixrendered
                            );
                        }else {
                            this.ptTile[this.sprTile[i]+256].render(this.buffer, 0, this.srcy1, 8, this.srcy2, this.sprX[i], this.sprY[i]+1, this.sprCol[i], this.sprPalette, this.horiFlip[i], this.vertFlip[i], i, this.pixrendered);
                        }
                    }else {
                        // 8x16 sprites
                        var top = this.sprTile[i];
                        if ((top&1)!==0) {
                            top = this.sprTile[i]-1+256;
                        }

                        var srcy1 = 0;
                        var srcy2 = 8;

                        if (this.sprY[i]<startscan) {
                            srcy1 = startscan - this.sprY[i]-1;
                        }

                        if (this.sprY[i]+8 > startscan+scancount) {
                            srcy2 = startscan+scancount-this.sprY[i];
                        }

                        this.ptTile[top+(this.vertFlip[i]?1:0)].render(
                            this.buffer,
                            0,
                            srcy1,
                            8,
                            srcy2,
                            this.sprX[i],
                            this.sprY[i]+1,
                            this.sprCol[i],
                            this.sprPalette,
                            this.horiFlip[i],
                            this.vertFlip[i],
                            i,
                            this.pixrendered
                        );

                        srcy1 = 0;
                        srcy2 = 8;

                        if (this.sprY[i]+8<startscan) {
                            srcy1 = startscan - (this.sprY[i]+8+1);
                        }

                        if (this.sprY[i]+16 > startscan+scancount) {
                            srcy2 = startscan+scancount-(this.sprY[i]+8);
                        }

                        this.ptTile[top+(this.vertFlip[i]?0:1)].render(
                            this.buffer,
                            0,
                            srcy1,
                            8,
                            srcy2,
                            this.sprX[i],
                            this.sprY[i]+1+8,
                            this.sprCol[i],
                            this.sprPalette,
                            this.horiFlip[i],
                            this.vertFlip[i],
                            i,
                            this.pixrendered
                        );

                    }
                }
            }
        }
    },

    checkSprite0: function(scan){

        this.spr0HitX = -1;
        this.spr0HitY = -1;

        var toffset;
        var tIndexAdd = (this.f_spPatternTable === 0?0:256);
        var x, y, t, i;
        var bufferIndex;
        var col;
        var bgPri;

        x = this.sprX[0];
        y = this.sprY[0]+1;

        if (this.f_spriteSize === 0) {
            // 8x8 sprites.

            // Check range:
            if (y <= scan && y + 8 > scan && x >= -7 && x < 256) {

                // Sprite is in range.
                // Draw scanline:
                t = this.ptTile[this.sprTile[0] + tIndexAdd];
                col = this.sprCol[0];
                bgPri = this.bgPriority[0];

                if (this.vertFlip[0]) {
                    toffset = 7 - (scan -y);
                }
                else {
                    toffset = scan - y;
                }
                toffset *= 8;

                bufferIndex = scan * 256 + x;
                if (this.horiFlip[0]) {
                    for (i = 7; i >= 0; i--) {
                        if (x >= 0 && x < 256) {
                            if (bufferIndex>=0 && bufferIndex<61440 &&
                                    this.pixrendered[bufferIndex] !==0 ) {
                                if (t.pix[toffset+i] !== 0) {
                                    this.spr0HitX = bufferIndex % 256;
                                    this.spr0HitY = scan;
                                    return true;
                                }
                            }
                        }
                        x++;
                        bufferIndex++;
                    }
                }
                else {
                    for (i = 0; i < 8; i++) {
                        if (x >= 0 && x < 256) {
                            if (bufferIndex >= 0 && bufferIndex < 61440 &&
                                    this.pixrendered[bufferIndex] !==0 ) {
                                if (t.pix[toffset+i] !== 0) {
                                    this.spr0HitX = bufferIndex % 256;
                                    this.spr0HitY = scan;
                                    return true;
                                }
                            }
                        }
                        x++;
                        bufferIndex++;
                    }
                }
            }
        }
        else {
            // 8x16 sprites:

            // Check range:
            if (y <= scan && y + 16 > scan && x >= -7 && x < 256) {
                // Sprite is in range.
                // Draw scanline:

                if (this.vertFlip[0]) {
                    toffset = 15-(scan-y);
                }else {
                    toffset = scan-y;
                }

                if (toffset<8) {
                    // first half of sprite.
                    t = this.ptTile[this.sprTile[0]+(this.vertFlip[0]?1:0)+((this.sprTile[0]&1)!==0?255:0)];
                }else {
                    // second half of sprite.
                    t = this.ptTile[this.sprTile[0]+(this.vertFlip[0]?0:1)+((this.sprTile[0]&1)!==0?255:0)];
                    if (this.vertFlip[0]) {
                        toffset = 15-toffset;
                    }
                    else {
                        toffset -= 8;
                    }
                }
                toffset*=8;
                col = this.sprCol[0];
                bgPri = this.bgPriority[0];

                bufferIndex = scan*256+x;
                if (this.horiFlip[0]) {
                    for (i=7;i>=0;i--) {
                        if (x>=0 && x<256) {
                            if (bufferIndex>=0 && bufferIndex<61440 && this.pixrendered[bufferIndex]!==0) {
                                if (t.pix[toffset+i] !== 0) {
                                    this.spr0HitX = bufferIndex%256;
                                    this.spr0HitY = scan;
                                    return true;
                                }
                            }
                        }
                        x++;
                        bufferIndex++;
                    }

                }
                else {

                    for (i=0;i<8;i++) {
                        if (x>=0 && x<256) {
                            if (bufferIndex>=0 && bufferIndex<61440 && this.pixrendered[bufferIndex]!==0) {
                                if (t.pix[toffset+i] !== 0) {
                                    this.spr0HitX = bufferIndex%256;
                                    this.spr0HitY = scan;
                                    return true;
                                }
                            }
                        }
                        x++;
                        bufferIndex++;
                    }

                }

            }

        }

        return false;
    },

    // This will write to PPU memory, and
    // update internally buffered data
    // appropriately.
    writeMem: function(address, value){
        this.vramMem[address] = value;

        // Update internally buffered data:
        if (address < 0x2000) {
            this.vramMem[address] = value;
            this.patternWrite(address,value);
        }
        else if (address >=0x2000 && address <0x23c0) {
            this.nameTableWrite(this.ntable1[0], address - 0x2000, value);
        }
        else if (address >=0x23c0 && address <0x2400) {
            this.attribTableWrite(this.ntable1[0],address-0x23c0,value);
        }
        else if (address >=0x2400 && address <0x27c0) {
            this.nameTableWrite(this.ntable1[1],address-0x2400,value);
        }
        else if (address >=0x27c0 && address <0x2800) {
            this.attribTableWrite(this.ntable1[1],address-0x27c0,value);
        }
        else if (address >=0x2800 && address <0x2bc0) {
            this.nameTableWrite(this.ntable1[2],address-0x2800,value);
        }
        else if (address >=0x2bc0 && address <0x2c00) {
            this.attribTableWrite(this.ntable1[2],address-0x2bc0,value);
        }
        else if (address >=0x2c00 && address <0x2fc0) {
            this.nameTableWrite(this.ntable1[3],address-0x2c00,value);
        }
        else if (address >=0x2fc0 && address <0x3000) {
            this.attribTableWrite(this.ntable1[3],address-0x2fc0,value);
        }
        else if (address >=0x3f00 && address <0x3f20) {
            this.updatePalettes();
        }
    },

    // Reads data from $3f00 to $f20
    // into the two buffered palettes.
    updatePalettes: function(){
        var i;

        for (i = 0; i < 16; i++) {
            if (this.f_dispType === 0) {
                this.imgPalette[i] = this.palTable.getEntry(
                    this.vramMem[0x3f00 + i] & 63
                );
            }
            else {
                this.imgPalette[i] = this.palTable.getEntry(
                    this.vramMem[0x3f00 + i] & 32
                );
            }
        }
        for (i = 0; i < 16; i++) {
            if (this.f_dispType === 0) {
                this.sprPalette[i] = this.palTable.getEntry(
                    this.vramMem[0x3f10 + i] & 63
                );
            }
            else {
                this.sprPalette[i] = this.palTable.getEntry(
                    this.vramMem[0x3f10 + i] & 32
                );
            }
        }
    },

    // Updates the internal pattern
    // table buffers with this new byte.
    // In vNES, there is a version of this with 4 arguments which isn't used.
    patternWrite: function(address, value){
        var tileIndex = Math.floor(address / 16);
        var leftOver = address%16;
        if (leftOver<8) {
            this.ptTile[tileIndex].setScanline(
                leftOver,
                value,
                this.vramMem[address+8]
            );
        }
        else {
            this.ptTile[tileIndex].setScanline(
                leftOver-8,
                this.vramMem[address-8],
                value
            );
        }
    },

    // Updates the internal name table buffers
    // with this new byte.
    nameTableWrite: function(index, address, value){
        this.nameTable[index].tile[address] = value;

        // Update Sprite #0 hit:
        //updateSpr0Hit();
        this.checkSprite0(this.scanline-20);
    },

    // Updates the internal pattern
    // table buffers with this new attribute
    // table byte.
    attribTableWrite: function(index, address, value){
        this.nameTable[index].writeAttrib(address,value);
    },

    // Updates the internally buffered sprite
    // data with this new byte of info.
    spriteRamWriteUpdate: function(address, value) {
        var tIndex = Math.floor(address / 4);

        if (tIndex === 0) {
            //updateSpr0Hit();
            this.checkSprite0(this.scanline - 20);
        }

        if (address % 4 === 0) {
            // Y coordinate
            this.sprY[tIndex] = value;
        }
        else if (address % 4 == 1) {
            // Tile index
            this.sprTile[tIndex] = value;
        }
        else if (address % 4 == 2) {
            // Attributes
            this.vertFlip[tIndex] = ((value & 0x80) !== 0);
            this.horiFlip[tIndex] = ((value & 0x40) !==0 );
            this.bgPriority[tIndex] = ((value & 0x20) !== 0);
            this.sprCol[tIndex] = (value & 3) << 2;

        }
        else if (address % 4 == 3) {
            // X coordinate
            this.sprX[tIndex] = value;
        }
    },

    doNMI: function() {
        // Set VBlank flag:
        this.setStatusFlag(this.STATUS_VBLANK,true);
        //nes.getCpu().doNonMaskableInterrupt();
        this.nes.cpu.requestIrq(this.nes.cpu.IRQ_NMI);
    },

    JSON_PROPERTIES: [
        // Memory
        'vramMem', 'spriteMem',
        // Counters
        'cntFV', 'cntV', 'cntH', 'cntVT', 'cntHT',
        // Registers
        'regFV', 'regV', 'regH', 'regVT', 'regHT', 'regFH', 'regS',
        // VRAM addr
        'vramAddress', 'vramTmpAddress',
        // Control/Status registers
        'f_nmiOnVblank', 'f_spriteSize', 'f_bgPatternTable', 'f_spPatternTable',
        'f_addrInc', 'f_nTblAddress', 'f_color', 'f_spVisibility',
        'f_bgVisibility', 'f_spClipping', 'f_bgClipping', 'f_dispType',
        // VRAM I/O
        'vramBufferedReadValue', 'firstWrite',
        // Mirroring
        'currentMirroring', 'vramMirrorTable', 'ntable1',
        // SPR-RAM I/O
        'sramAddress',
        // Sprites. Most sprite data is rebuilt from spriteMem
        'hitSpr0',
        // Palettes
        'sprPalette', 'imgPalette',
        // Rendering progression
        'curX', 'scanline', 'lastRenderedScanline', 'curNt', 'scantile',
        // Used during rendering
        'attrib', 'buffer', 'bgbuffer', 'pixrendered',
        // Misc
        'requestEndFrame', 'nmiOk', 'dummyCycleToggle', 'nmiCounter',
        'validTileData', 'scanlineAlreadyRendered'
    ],

    toJSON: function() {
        var i;
        var state = JSNES.Utils.toJSON(this);

        state.nameTable = [];
        for (i = 0; i < this.nameTable.length; i++) {
            state.nameTable[i] = this.nameTable[i].toJSON();
        }

        state.ptTile = [];
        for (i = 0; i < this.ptTile.length; i++) {
            state.ptTile[i] = this.ptTile[i].toJSON();
        }

        return state;
    },

    fromJSON: function(state) {
        var i;

        JSNES.Utils.fromJSON(this, state);

        for (i = 0; i < this.nameTable.length; i++) {
            this.nameTable[i].fromJSON(state.nameTable[i]);
        }

        for (i = 0; i < this.ptTile.length; i++) {
            this.ptTile[i].fromJSON(state.ptTile[i]);
        }

        // Sprite data:
        for (i = 0; i < this.spriteMem.length; i++) {
            this.spriteRamWriteUpdate(i, this.spriteMem[i]);
        }
    }
};

JSNES.PPU.NameTable = function(width, height, name) {
    this.width = width;
    this.height = height;
    this.name = name;

    this.tile = new Array(width*height);
    this.attrib = new Array(width*height);
    for ( i = 0; i < width*height; i++ )
    {
        this.tile[i] = 0;
        this.attrib[i] = 0;
    }
};

JSNES.PPU.NameTable.prototype = {
    getTileIndex: function(x, y){
        return this.tile[y*this.width+x];
    },

    getAttrib: function(x, y){
        return this.attrib[y*this.width+x];
    },

    writeAttrib: function(index, value){
        var basex = (index % 8) * 4;
        var basey = Math.floor(index / 8) * 4;
        var add;
        var tx, ty;
        var attindex;

        for (var sqy=0;sqy<2;sqy++) {
            for (var sqx=0;sqx<2;sqx++) {
                add = (value>>(2*(sqy*2+sqx)))&3;
                for (var y=0;y<2;y++) {
                    for (var x=0;x<2;x++) {
                        tx = basex+sqx*2+x;
                        ty = basey+sqy*2+y;
                        attindex = ty*this.width+tx;
                        this.attrib[ty*this.width+tx] = (add<<2)&12;
                    }
                }
            }
        }
    },

    toJSON: function() {
        return {
            'tile': this.tile,
            'attrib': this.attrib
        };
    },

    fromJSON: function(s) {
        this.tile = s.tile;
        this.attrib = s.attrib;
    }
};


JSNES.PPU.PaletteTable = function() {
    this.curTable = new Array(64);
    this.emphTable = new Array(8);
    this.currentEmph = -1;
};

JSNES.PPU.PaletteTable.prototype = {
    reset: function() {
        this.setEmphasis(0);
    },

    loadNTSCPalette: function() {
        this.curTable = [0x525252, 0xB40000, 0xA00000, 0xB1003D, 0x740069, 0x00005B, 0x00005F, 0x001840, 0x002F10, 0x084A08, 0x006700, 0x124200, 0x6D2800, 0x000000, 0x000000, 0x000000, 0xC4D5E7, 0xFF4000, 0xDC0E22, 0xFF476B, 0xD7009F, 0x680AD7, 0x0019BC, 0x0054B1, 0x006A5B, 0x008C03, 0x00AB00, 0x2C8800, 0xA47200, 0x000000, 0x000000, 0x000000, 0xF8F8F8, 0xFFAB3C, 0xFF7981, 0xFF5BC5, 0xFF48F2, 0xDF49FF, 0x476DFF, 0x00B4F7, 0x00E0FF, 0x00E375, 0x03F42B, 0x78B82E, 0xE5E218, 0x787878, 0x000000, 0x000000, 0xFFFFFF, 0xFFF2BE, 0xF8B8B8, 0xF8B8D8, 0xFFB6FF, 0xFFC3FF, 0xC7D1FF, 0x9ADAFF, 0x88EDF8, 0x83FFDD, 0xB8F8B8, 0xF5F8AC, 0xFFFFB0, 0xF8D8F8, 0x000000, 0x000000];
        this.makeTables();
        this.setEmphasis(0);
    },

    loadPALPalette: function() {
        this.curTable = [0x525252, 0xB40000, 0xA00000, 0xB1003D, 0x740069, 0x00005B, 0x00005F, 0x001840, 0x002F10, 0x084A08, 0x006700, 0x124200, 0x6D2800, 0x000000, 0x000000, 0x000000, 0xC4D5E7, 0xFF4000, 0xDC0E22, 0xFF476B, 0xD7009F, 0x680AD7, 0x0019BC, 0x0054B1, 0x006A5B, 0x008C03, 0x00AB00, 0x2C8800, 0xA47200, 0x000000, 0x000000, 0x000000, 0xF8F8F8, 0xFFAB3C, 0xFF7981, 0xFF5BC5, 0xFF48F2, 0xDF49FF, 0x476DFF, 0x00B4F7, 0x00E0FF, 0x00E375, 0x03F42B, 0x78B82E, 0xE5E218, 0x787878, 0x000000, 0x000000, 0xFFFFFF, 0xFFF2BE, 0xF8B8B8, 0xF8B8D8, 0xFFB6FF, 0xFFC3FF, 0xC7D1FF, 0x9ADAFF, 0x88EDF8, 0x83FFDD, 0xB8F8B8, 0xF5F8AC, 0xFFFFB0, 0xF8D8F8, 0x000000, 0x000000];
        this.makeTables();
        this.setEmphasis(0);
    },

    makeTables: function(){
        var r, g, b, col, i, rFactor, gFactor, bFactor;

        // Calculate a table for each possible emphasis setting:
        for (var emph = 0; emph < 8; emph++) {

            // Determine color component factors:
            rFactor = 1.0;
            gFactor = 1.0;
            bFactor = 1.0;

            if ((emph & 1) !== 0) {
                rFactor = 0.75;
                bFactor = 0.75;
            }
            if ((emph & 2) !== 0) {
                rFactor = 0.75;
                gFactor = 0.75;
            }
            if ((emph & 4) !== 0) {
                gFactor = 0.75;
                bFactor = 0.75;
            }

            this.emphTable[emph] = new Array(64);

            // Calculate table:
            for (i = 0; i < 64; i++) {
                col = this.curTable[i];
                r = Math.floor(this.getRed(col) * rFactor);
                g = Math.floor(this.getGreen(col) * gFactor);
                b = Math.floor(this.getBlue(col) * bFactor);
                this.emphTable[emph][i] = this.getRgb(r, g, b);
            }
        }
    },

    setEmphasis: function(emph){
        if (emph != this.currentEmph) {
            this.currentEmph = emph;
            for (var i = 0; i < 64; i++) {
                this.curTable[i] = this.emphTable[emph][i];
            }
        }
    },

    getEntry: function(yiq){
        return this.curTable[yiq];
    },

    getRed: function(rgb){
        return (rgb>>16)&0xFF;
    },

    getGreen: function(rgb){
        return (rgb>>8)&0xFF;
    },

    getBlue: function(rgb){
        return rgb&0xFF;
    },

    getRgb: function(r, g, b){
        return ((r<<16)|(g<<8)|(b));
    },

    loadDefaultPalette: function(){
        this.curTable[ 0] = this.getRgb(117,117,117);
        this.curTable[ 1] = this.getRgb( 39, 27,143);
        this.curTable[ 2] = this.getRgb(  0,  0,171);
        this.curTable[ 3] = this.getRgb( 71,  0,159);
        this.curTable[ 4] = this.getRgb(143,  0,119);
        this.curTable[ 5] = this.getRgb(171,  0, 19);
        this.curTable[ 6] = this.getRgb(167,  0,  0);
        this.curTable[ 7] = this.getRgb(127, 11,  0);
        this.curTable[ 8] = this.getRgb( 67, 47,  0);
        this.curTable[ 9] = this.getRgb(  0, 71,  0);
        this.curTable[10] = this.getRgb(  0, 81,  0);
        this.curTable[11] = this.getRgb(  0, 63, 23);
        this.curTable[12] = this.getRgb( 27, 63, 95);
        this.curTable[13] = this.getRgb(  0,  0,  0);
        this.curTable[14] = this.getRgb(  0,  0,  0);
        this.curTable[15] = this.getRgb(  0,  0,  0);
        this.curTable[16] = this.getRgb(188,188,188);
        this.curTable[17] = this.getRgb(  0,115,239);
        this.curTable[18] = this.getRgb( 35, 59,239);
        this.curTable[19] = this.getRgb(131,  0,243);
        this.curTable[20] = this.getRgb(191,  0,191);
        this.curTable[21] = this.getRgb(231,  0, 91);
        this.curTable[22] = this.getRgb(219, 43,  0);
        this.curTable[23] = this.getRgb(203, 79, 15);
        this.curTable[24] = this.getRgb(139,115,  0);
        this.curTable[25] = this.getRgb(  0,151,  0);
        this.curTable[26] = this.getRgb(  0,171,  0);
        this.curTable[27] = this.getRgb(  0,147, 59);
        this.curTable[28] = this.getRgb(  0,131,139);
        this.curTable[29] = this.getRgb(  0,  0,  0);
        this.curTable[30] = this.getRgb(  0,  0,  0);
        this.curTable[31] = this.getRgb(  0,  0,  0);
        this.curTable[32] = this.getRgb(255,255,255);
        this.curTable[33] = this.getRgb( 63,191,255);
        this.curTable[34] = this.getRgb( 95,151,255);
        this.curTable[35] = this.getRgb(167,139,253);
        this.curTable[36] = this.getRgb(247,123,255);
        this.curTable[37] = this.getRgb(255,119,183);
        this.curTable[38] = this.getRgb(255,119, 99);
        this.curTable[39] = this.getRgb(255,155, 59);
        this.curTable[40] = this.getRgb(243,191, 63);
        this.curTable[41] = this.getRgb(131,211, 19);
        this.curTable[42] = this.getRgb( 79,223, 75);
        this.curTable[43] = this.getRgb( 88,248,152);
        this.curTable[44] = this.getRgb(  0,235,219);
        this.curTable[45] = this.getRgb(  0,  0,  0);
        this.curTable[46] = this.getRgb(  0,  0,  0);
        this.curTable[47] = this.getRgb(  0,  0,  0);
        this.curTable[48] = this.getRgb(255,255,255);
        this.curTable[49] = this.getRgb(171,231,255);
        this.curTable[50] = this.getRgb(199,215,255);
        this.curTable[51] = this.getRgb(215,203,255);
        this.curTable[52] = this.getRgb(255,199,255);
        this.curTable[53] = this.getRgb(255,199,219);
        this.curTable[54] = this.getRgb(255,191,179);
        this.curTable[55] = this.getRgb(255,219,171);
        this.curTable[56] = this.getRgb(255,231,163);
        this.curTable[57] = this.getRgb(227,255,163);
        this.curTable[58] = this.getRgb(171,243,191);
        this.curTable[59] = this.getRgb(179,255,207);
        this.curTable[60] = this.getRgb(159,255,243);
        this.curTable[61] = this.getRgb(  0,  0,  0);
        this.curTable[62] = this.getRgb(  0,  0,  0);
        this.curTable[63] = this.getRgb(  0,  0,  0);

        this.makeTables();
        this.setEmphasis(0);
    }
};

JSNES.PPU.Tile = function() {
    // Tile data:
    this.pix = new Array(64);

    this.fbIndex = null;
    this.tIndex = null;
    this.x = null;
    this.y = null;
    this.w = null;
    this.h = null;
    this.incX = null;
    this.incY = null;
    this.palIndex = null;
    this.tpri = null;
    this.c = null;
    this.initialized = false;
    this.opaque = new Array(8);
};

JSNES.PPU.Tile.prototype = {
    setBuffer: function(scanline){
        for (this.y=0;this.y<8;this.y++) {
            this.setScanline(this.y,scanline[this.y],scanline[this.y+8]);
        }
    },

    setScanline: function(sline, b1, b2){
        this.initialized = true;
        this.tIndex = sline<<3;
        for (this.x = 0; this.x < 8; this.x++) {
            this.pix[this.tIndex + this.x] = ((b1 >> (7 - this.x)) & 1) +
                    (((b2 >> (7 - this.x)) & 1) << 1);
            if(this.pix[this.tIndex+this.x] === 0) {
                this.opaque[sline] = false;
            }
        }
    },

    render: function(buffer, srcx1, srcy1, srcx2, srcy2, dx, dy, palAdd, palette, flipHorizontal, flipVertical, pri, priTable) {

        if (dx<-7 || dx>=256 || dy<-7 || dy>=240) {
            return;
        }

        this.w=srcx2-srcx1;
        this.h=srcy2-srcy1;

        if (dx<0) {
            srcx1-=dx;
        }
        if (dx+srcx2>=256) {
            srcx2=256-dx;
        }

        if (dy<0) {
            srcy1-=dy;
        }
        if (dy+srcy2>=240) {
            srcy2=240-dy;
        }

        if (!flipHorizontal && !flipVertical) {

            this.fbIndex = (dy<<8)+dx;
            this.tIndex = 0;
            for (this.y=0;this.y<8;this.y++) {
                for (this.x=0;this.x<8;this.x++) {
                    if (this.x>=srcx1 && this.x<srcx2 && this.y>=srcy1 && this.y<srcy2) {
                        this.palIndex = this.pix[this.tIndex];
                        this.tpri = priTable[this.fbIndex];
                        if (this.palIndex!==0 && pri<=(this.tpri&0xFF)) {
                            //console.log("Rendering upright tile to buffer");
                            buffer[this.fbIndex] = palette[this.palIndex+palAdd];
                            this.tpri = (this.tpri&0xF00)|pri;
                            priTable[this.fbIndex] =this.tpri;
                        }
                    }
                    this.fbIndex++;
                    this.tIndex++;
                }
                this.fbIndex-=8;
                this.fbIndex+=256;
            }

        }else if (flipHorizontal && !flipVertical) {

            this.fbIndex = (dy<<8)+dx;
            this.tIndex = 7;
            for (this.y=0;this.y<8;this.y++) {
                for (this.x=0;this.x<8;this.x++) {
                    if (this.x>=srcx1 && this.x<srcx2 && this.y>=srcy1 && this.y<srcy2) {
                        this.palIndex = this.pix[this.tIndex];
                        this.tpri = priTable[this.fbIndex];
                        if (this.palIndex!==0 && pri<=(this.tpri&0xFF)) {
                            buffer[this.fbIndex] = palette[this.palIndex+palAdd];
                            this.tpri = (this.tpri&0xF00)|pri;
                            priTable[this.fbIndex] =this.tpri;
                        }
                    }
                    this.fbIndex++;
                    this.tIndex--;
                }
                this.fbIndex-=8;
                this.fbIndex+=256;
                this.tIndex+=16;
            }

        }
        else if(flipVertical && !flipHorizontal) {

            this.fbIndex = (dy<<8)+dx;
            this.tIndex = 56;
            for (this.y=0;this.y<8;this.y++) {
                for (this.x=0;this.x<8;this.x++) {
                    if (this.x>=srcx1 && this.x<srcx2 && this.y>=srcy1 && this.y<srcy2) {
                        this.palIndex = this.pix[this.tIndex];
                        this.tpri = priTable[this.fbIndex];
                        if (this.palIndex!==0 && pri<=(this.tpri&0xFF)) {
                            buffer[this.fbIndex] = palette[this.palIndex+palAdd];
                            this.tpri = (this.tpri&0xF00)|pri;
                            priTable[this.fbIndex] =this.tpri;
                        }
                    }
                    this.fbIndex++;
                    this.tIndex++;
                }
                this.fbIndex-=8;
                this.fbIndex+=256;
                this.tIndex-=16;
            }

        }
        else {
            this.fbIndex = (dy<<8)+dx;
            this.tIndex = 63;
            for (this.y=0;this.y<8;this.y++) {
                for (this.x=0;this.x<8;this.x++) {
                    if (this.x>=srcx1 && this.x<srcx2 && this.y>=srcy1 && this.y<srcy2) {
                        this.palIndex = this.pix[this.tIndex];
                        this.tpri = priTable[this.fbIndex];
                        if (this.palIndex!==0 && pri<=(this.tpri&0xFF)) {
                            buffer[this.fbIndex] = palette[this.palIndex+palAdd];
                            this.tpri = (this.tpri&0xF00)|pri;
                            priTable[this.fbIndex] =this.tpri;
                        }
                    }
                    this.fbIndex++;
                    this.tIndex--;
                }
                this.fbIndex-=8;
                this.fbIndex+=256;
            }

        }

    },

    isTransparent: function(x, y){
        return (this.pix[(y << 3) + x] === 0);
    },

    toJSON: function() {
        return {
            'opaque': this.opaque,
            'pix': this.pix
        };
    },

    fromJSON: function(s) {
        this.opaque = s.opaque;
        this.pix = s.pix;
    }
};
/*
JSNES, based on Jamie Sanders' vNES
Copyright (C) 2010 Ben Firshman

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

JSNES.ROM = function(nes) {
    this.nes = nes;

    this.mapperName = new Array(92);

    for (var i=0;i<92;i++) {
        this.mapperName[i] = "Unknown Mapper";
    }
    this.mapperName[ 0] = "Direct Access";
    this.mapperName[ 1] = "Nintendo MMC1";
    this.mapperName[ 2] = "UNROM";
    this.mapperName[ 3] = "CNROM";
    this.mapperName[ 4] = "Nintendo MMC3";
    this.mapperName[ 5] = "Nintendo MMC5";
    this.mapperName[ 6] = "FFE F4xxx";
    this.mapperName[ 7] = "AOROM";
    this.mapperName[ 8] = "FFE F3xxx";
    this.mapperName[ 9] = "Nintendo MMC2";
    this.mapperName[10] = "Nintendo MMC4";
    this.mapperName[11] = "Color Dreams Chip";
    this.mapperName[12] = "FFE F6xxx";
    this.mapperName[15] = "100-in-1 switch";
    this.mapperName[16] = "Bandai chip";
    this.mapperName[17] = "FFE F8xxx";
    this.mapperName[18] = "Jaleco SS8806 chip";
    this.mapperName[19] = "Namcot 106 chip";
    this.mapperName[20] = "Famicom Disk System";
    this.mapperName[21] = "Konami VRC4a";
    this.mapperName[22] = "Konami VRC2a";
    this.mapperName[23] = "Konami VRC2a";
    this.mapperName[24] = "Konami VRC6";
    this.mapperName[25] = "Konami VRC4b";
    this.mapperName[32] = "Irem G-101 chip";
    this.mapperName[33] = "Taito TC0190/TC0350";
    this.mapperName[34] = "32kB ROM switch";

    this.mapperName[64] = "Tengen RAMBO-1 chip";
    this.mapperName[65] = "Irem H-3001 chip";
    this.mapperName[66] = "GNROM switch";
    this.mapperName[67] = "SunSoft3 chip";
    this.mapperName[68] = "SunSoft4 chip";
    this.mapperName[69] = "SunSoft5 FME-7 chip";
    this.mapperName[71] = "Camerica chip";
    this.mapperName[78] = "Irem 74HC161/32-based";
    this.mapperName[91] = "Pirate HK-SF3 chip";
};

JSNES.ROM.prototype = {
    // Mirroring types:
    VERTICAL_MIRRORING: 0,
    HORIZONTAL_MIRRORING: 1,
    FOURSCREEN_MIRRORING: 2,
    SINGLESCREEN_MIRRORING: 3,
    SINGLESCREEN_MIRRORING2: 4,
    SINGLESCREEN_MIRRORING3: 5,
    SINGLESCREEN_MIRRORING4: 6,
    CHRROM_MIRRORING: 7,

    header: null,
    rom: null,
    vrom: null,
    vromTile: null,

    romCount: null,
    vromCount: null,
    mirroring: null,
    batteryRam: null,
    trainer: null,
    fourScreen: null,
    mapperType: null,
    valid: false,

    load: function(data) {
        var i, j, v;

        if (data.indexOf("NES\x1a") === -1) {
            this.nes.ui.updateStatus("Not a valid NES ROM.");
            return;
        }
        this.header = new Array(16);
        for (i = 0; i < 16; i++) {
            this.header[i] = data.charCodeAt(i) & 0xFF;
        }
        this.romCount = this.header[4];
        this.vromCount = this.header[5]*2; // Get the number of 4kB banks, not 8kB
        this.mirroring = ((this.header[6] & 1) !== 0 ? 1 : 0);
        this.batteryRam = (this.header[6] & 2) !== 0;
        this.trainer = (this.header[6] & 4) !== 0;
        this.fourScreen = (this.header[6] & 8) !== 0;
        this.mapperType = (this.header[6] >> 4) | (this.header[7] & 0xF0);
        /* TODO
        if (this.batteryRam)
            this.loadBatteryRam();*/
        // Check whether byte 8-15 are zero's:
        var foundError = false;
        for (i=8; i<16; i++) {
            if (this.header[i] !== 0) {
                foundError = true;
                break;
            }
        }
        if (foundError) {
            this.mapperType &= 0xF; // Ignore byte 7
        }
        // Load PRG-ROM banks:
        this.rom = new Array(this.romCount);
        var offset = 16;
        for (i=0; i < this.romCount; i++) {
            this.rom[i] = new Array(16384);
            for (j=0; j < 16384; j++) {
                if (offset+j >= data.length) {
                    break;
                }
                this.rom[i][j] = data.charCodeAt(offset + j) & 0xFF;
            }
            offset += 16384;
        }
        // Load CHR-ROM banks:
        this.vrom = new Array(this.vromCount);
        for (i=0; i < this.vromCount; i++) {
            this.vrom[i] = new Array(4096);
            for (j=0; j < 4096; j++) {
                if (offset+j >= data.length){
                    break;
                }
                this.vrom[i][j] = data.charCodeAt(offset + j) & 0xFF;
            }
            offset += 4096;
        }

        // Create VROM tiles:
        this.vromTile = new Array(this.vromCount);
        for (i=0; i < this.vromCount; i++) {
            this.vromTile[i] = new Array(256);
            for (j=0; j < 256; j++) {
                this.vromTile[i][j] = new JSNES.PPU.Tile();
            }
        }

        // Convert CHR-ROM banks to tiles:
        var tileIndex;
        var leftOver;
        for (v=0; v < this.vromCount; v++) {
            for (i=0; i < 4096; i++) {
                tileIndex = i >> 4;
                leftOver = i % 16;
                if (leftOver < 8) {
                    this.vromTile[v][tileIndex].setScanline(
                        leftOver,
                        this.vrom[v][i],
                        this.vrom[v][i+8]
                    );
                }
                else {
                    this.vromTile[v][tileIndex].setScanline(
                        leftOver-8,
                        this.vrom[v][i-8],
                        this.vrom[v][i]
                    );
                }
            }
        }

        this.valid = true;
    },

    getMirroringType: function() {
        if (this.fourScreen) {
            return this.FOURSCREEN_MIRRORING;
        }
        if (this.mirroring === 0) {
            return this.HORIZONTAL_MIRRORING;
        }
        return this.VERTICAL_MIRRORING;
    },

    getMapperName: function() {
        if (this.mapperType >= 0 && this.mapperType < this.mapperName.length) {
            return this.mapperName[this.mapperType];
        }
        return "Unknown Mapper, "+this.mapperType;
    },

    mapperSupported: function() {
        return typeof JSNES.Mappers[this.mapperType] !== 'undefined';
    },

    createMapper: function() {
        if (this.mapperSupported()) {
            return new JSNES.Mappers[this.mapperType](this.nes);
        }
        else {
            this.nes.ui.updateStatus("This ROM uses a mapper not supported by JSNES: "+this.getMapperName()+"("+this.mapperType+")");
            return null;
        }
    }
};
/*
JSNES, based on Jamie Sanders' vNES
Copyright (C) 2010 Ben Firshman

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

JSNES.DummyUI = function(nes) {
    this.nes = nes;
    this.enable = function() {};
    this.updateStatus = function() {};
    this.writeAudio = function() {};
    this.writeFrame = function() {};
};

if (typeof jQuery !== 'undefined') {
    (function($) {
        $.fn.JSNESUI = function(roms) {
            var parent = this;
            var UI = function(nes) {
                var self = this;
                self.nes = nes;

                /*
                 * Create UI
                 */
                self.root = $('<div></div>');
                self.screen = $('<canvas class="nes-screen" width="256" height="240"></canvas>').appendTo(self.root);

                if (!self.screen[0].getContext) {
                    parent.html("Your browser doesn't support the <code>&lt;canvas&gt;</code> tag. Try Google Chrome, Safari, Opera or Firefox!");
                    return;
                }

                self.romContainer = $('<div class="nes-roms"></div>').appendTo(self.root);
                self.controls = $('<div class="nes-controls"></div>').appendTo(self.root);
                self.root.appendTo(parent);

                this.nes.opts.emulateSound = false;

                if (typeof roms != 'undefined') {
                    this.loadROM(roms);
                }

                /*
                 * Canvas
                 */
                self.canvasContext = self.screen[0].getContext('2d');

                if (!self.canvasContext.getImageData) {
                    parent.html("Your browser doesn't support writing pixels directly to the <code>&lt;canvas&gt;</code> tag. Try the latest versions of Google Chrome, Safari, Opera or Firefox!");
                    return;
                }

                self.canvasImageData = self.canvasContext.getImageData(0, 0, 256, 240);
                self.resetCanvas();

                /*
                 * Keyboard
                 */
                $(document).
                    bind('keydown', function(evt) {
                        self.nes.keyboard.keyDown(evt);
                    }).
                    bind('keyup', function(evt) {
                        self.nes.keyboard.keyUp(evt);
                    }).
                    bind('keypress', function(evt) {
                        self.nes.keyboard.keyPress(evt);
                    });

                /*
                 * Sound
                 */
                self.dynamicaudio = new DynamicAudio({
                    swf: nes.opts.swfPath+'dynamicaudio.swf'
                });
            };

            UI.prototype = {
                loadROM: function(url) {
                    var self = this;
                    self.updateStatus("Downloading...");
                    $.ajax({
                        url: url,
                        xhr: function() {
                            var xhr = $.ajaxSettings.xhr();
                            if (typeof xhr.overrideMimeType !== 'undefined') {
                                // Download as binary
                                xhr.overrideMimeType('text/plain; charset=x-user-defined');
                            }
                            self.xhr = xhr;
                            return xhr;
                        },
                        complete: function(xhr, status) {
                            var i, data;
                            if (JSNES.Utils.isIE()) {
                                var charCodes = JSNESBinaryToArray(
                                    xhr.responseBody
                                ).toArray();
                                data = String.fromCharCode.apply(
                                    undefined,
                                    charCodes
                                );
                            }
                            else {
                                data = xhr.responseText;
                            }
                            self.nes.loadRom(data);
                            self.nes.start();
                            self.enable();
                        }
                    });
                },

                resetCanvas: function() {
                    this.canvasContext.fillStyle = 'black';
                    // set alpha to opaque
                    this.canvasContext.fillRect(0, 0, 256, 240);

                    // Set alpha
                    for (var i = 3; i < this.canvasImageData.data.length-3; i += 4) {
                        this.canvasImageData.data[i] = 0xFF;
                    }
                },

                /*
                *
                * nes.ui.screenshot() --> return <img> element :)
                */
                screenshot: function() {
                    var data = this.screen[0].toDataURL("image/png"),
                        img = new Image();
                    img.src = data;
                    return img;
                },

                /*
                 * Enable and reset UI elements
                 */
                enable: function() {},

                updateStatus: function(s) {},

                writeAudio: function(samples) {
                    return this.dynamicaudio.writeInt(samples);
                },

                writeFrame: function(buffer, prevBuffer) {
                    var imageData = this.canvasImageData.data;
                    var pixel, i, j;

                    for (i=0; i<256*240; i++) {
                        pixel = buffer[i];

                        if (pixel != prevBuffer[i]) {
                            j = i*4;
                            imageData[j] = pixel & 0xFF;
                            imageData[j+1] = (pixel >> 8) & 0xFF;
                            imageData[j+2] = (pixel >> 16) & 0xFF;
                            prevBuffer[i] = pixel;
                        }
                    }

                    this.canvasContext.putImageData(this.canvasImageData, 0, 0);
                }
            };

            return UI;
        };
    })(jQuery);
}
