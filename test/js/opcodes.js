/**
 * OPCODES
 */
QUnit.module('opcodes');

/**
 * ADC
 */
QUnit.test('ADC', function (assert) {
    var done = assert.async(),
        input = [
            '    ADC #$44',
            '    ADC <$44',
            '    ADC <$44, X',
            '    ADC $4400',
            '    ADC $4400, X',
            '    ADC $4400, Y',
            '    ADC [$44, X]',
            '    ADC [$44], Y',
            '',
            'label:',
            '    ADC #label',
            '    ADC <label',
            '    ADC <label, X',
            '    ADC label',
            '    ADC label, X',
            '    ADC label, Y',
            '    ADC [label, X]',
            '    ADC [label], Y',
            '',
            'CONSTANT_8 = $44',
            'CONSTANT_16 = $4400',
            '',
            '    ADC #CONSTANT_8',
            '    ADC <CONSTANT_8',
            '    ADC <CONSTANT_8, X',
            '    ADC CONSTANT_16',
            '    ADC CONSTANT_16, X',
            '    ADC CONSTANT_16, Y',
            '    ADC [CONSTANT_8, X]',
            '    ADC [CONSTANT_8], Y'
        ],
        output = [
            0x69, 0x44, 0x65, 0x44, 0x75, 0x44, 0x6d, 0x00, 0x44, 0x7d, 0x00, 0x44, 0x79, 0x00, 0x44, 0x61,
            0x44, 0x71, 0x44, 0x69, 0x13, 0x65, 0x13, 0x75, 0x13, 0x6d, 0x13, 0x00, 0x7d, 0x13, 0x00, 0x79,
            0x13, 0x00, 0x61, 0x13, 0x71, 0x13, 0x69, 0x44, 0x65, 0x44, 0x75, 0x44, 0x6d, 0x00, 0x44, 0x7d,
            0x00, 0x44, 0x79, 0x00, 0x44, 0x61, 0x44, 0x71, 0x44
        ],
        nessemble = new Nessemble({
            stdin: input.join('\n'),
            onStdout: function (rc, stdout) {
                assert.deepEqual(stdout, output);
                done();
            }
        });

    nessemble.callMain();
});

/**
 * AND
 */
QUnit.test('AND', function (assert) {
    var done = assert.async(),
        input = [
            '    AND #$44',
            '    AND <$44',
            '    AND <$44, X',
            '    AND $4400',
            '    AND $4400, X',
            '    AND $4400, Y',
            '    AND [$44, X]',
            '    AND [$44], Y',
            '',
            'label:',
            '    AND #label',
            '    AND <label',
            '    AND <label, X',
            '    AND label',
            '    AND label, X',
            '    AND label, Y',
            '    AND [label, X]',
            '    AND [label], Y',
            '',
            'CONSTANT_8 = $44',
            'CONSTANT_16 = $4400',
            '',
            '    AND #CONSTANT_8',
            '    AND <CONSTANT_8',
            '    AND <CONSTANT_8, X',
            '    AND CONSTANT_16',
            '    AND CONSTANT_16, X',
            '    AND CONSTANT_16, Y',
            '    AND [CONSTANT_8, X]',
            '    AND [CONSTANT_8], Y'
        ],
        output = [
            0x29, 0x44, 0x25, 0x44, 0x35, 0x44, 0x2d, 0x00, 0x44, 0x3d, 0x00, 0x44, 0x39, 0x00, 0x44, 0x21,
            0x44, 0x31, 0x44, 0x29, 0x13, 0x25, 0x13, 0x35, 0x13, 0x2d, 0x13, 0x00, 0x3d, 0x13, 0x00, 0x39,
            0x13, 0x00, 0x21, 0x13, 0x31, 0x13, 0x29, 0x44, 0x25, 0x44, 0x35, 0x44, 0x2d, 0x00, 0x44, 0x3d,
            0x00, 0x44, 0x39, 0x00, 0x44, 0x21, 0x44, 0x31, 0x44
        ],
        nessemble = new Nessemble({
            stdin: input.join('\n'),
            onStdout: function (rc, stdout) {
                assert.deepEqual(stdout, output);
                done();
            }
        });

    nessemble.callMain();
});

/**
 * ASL
 */
QUnit.test('ASL', function (assert) {
    var done = assert.async(),
        input = [
            '    ASL A',
            '    ASL <$44',
            '    ASL <$44, X',
            '    ASL $4400',
            '    ASL $4400, X',
            '',
            'label:',
            '    ASL A',
            '    ASL <label',
            '    ASL <label, X',
            '    ASL label',
            '    ASL label, X',
            '',
            'CONSTANT_8 = $44',
            'CONSTANT_16 = $4400',
            '',
            '    ASL A',
            '    ASL <CONSTANT_8',
            '    ASL <CONSTANT_8, X',
            '    ASL CONSTANT_16',
            '    ASL CONSTANT_16, X'
        ],
        output = [
            0x0a, 0x06, 0x44, 0x16, 0x44, 0x0e, 0x00, 0x44, 0x1e, 0x00, 0x44, 0x0a, 0x06, 0x0b, 0x16, 0x0b,
            0x0e, 0x0b, 0x00, 0x1e, 0x0b, 0x00, 0x0a, 0x06, 0x44, 0x16, 0x44, 0x0e, 0x00, 0x44, 0x1e, 0x00,
            0x44
        ],
        nessemble = new Nessemble({
            stdin: input.join('\n'),
            onStdout: function (rc, stdout) {
                assert.deepEqual(stdout, output);
                done();
            }
        });

    nessemble.callMain();
});

/**
 * BCC
 */
QUnit.test('BCC', function (assert) {
    var done = assert.async(),
        input = [
            '    BCC $00',
            '',
            'label:',
            '    BCC label'
        ],
        output = [
            0x90, 0xfe, 0x90, 0xfe
        ],
        nessemble = new Nessemble({
            stdin: input.join('\n'),
            onStdout: function (rc, stdout) {
                assert.deepEqual(stdout, output);
                done();
            }
        });

    nessemble.callMain();
});

/**
 * BCS
 */
QUnit.test('BCS', function (assert) {
    var done = assert.async(),
        input = [
            '    BCS $00',
            '',
            'label:',
            '    BCS label'
        ],
        output = [
            0xb0, 0xfe, 0xb0, 0xfe
        ],
        nessemble = new Nessemble({
            stdin: input.join('\n'),
            onStdout: function (rc, stdout) {
                assert.deepEqual(stdout, output);
                done();
            }
        });

    nessemble.callMain();
});

/**
 * BEQ
 */
QUnit.test('BEQ', function (assert) {
    var done = assert.async(),
        input = [
            '    BEQ $00',
            '',
            'label:',
            '    BEQ label'
        ],
        output = [
            0xf0, 0xfe, 0xf0, 0xfe
        ],
        nessemble = new Nessemble({
            stdin: input.join('\n'),
            onStdout: function (rc, stdout) {
                assert.deepEqual(stdout, output);
                done();
            }
        });

    nessemble.callMain();
});

/**
 * BIT
 */
QUnit.test('BIT', function (assert) {
    var done = assert.async(),
        input = [
            '    BIT <$44',
            '    BIT $4400',
            '',
            'label:',
            '    BIT <label',
            '    BIT label',
            '',
            'CONSTANT_8 = $44',
            'CONSTANT_16 = $4400',
            '',
            '    BIT <CONSTANT_8',
            '    BIT CONSTANT_16'
        ],
        output = [
            0x24, 0x44, 0x2c, 0x00, 0x44, 0x24, 0x05, 0x2c, 0x05, 0x00, 0x24, 0x44, 0x2c, 0x00, 0x44
        ],
        nessemble = new Nessemble({
            stdin: input.join('\n'),
            onStdout: function (rc, stdout) {
                assert.deepEqual(stdout, output);
                done();
            }
        });

    nessemble.callMain();
});

/**
 * BMI
 */
QUnit.test('BMI', function (assert) {
    var done = assert.async(),
        input = [
            '    BMI $00',
            '',
            'label:',
            '    BMI label'
        ],
        output = [
            0x30, 0xfe, 0x30, 0xfe
        ],
        nessemble = new Nessemble({
            stdin: input.join('\n'),
            onStdout: function (rc, stdout) {
                assert.deepEqual(stdout, output);
                done();
            }
        });

    nessemble.callMain();
});

/**
 * BNE
 */
QUnit.test('BNE', function (assert) {
    var done = assert.async(),
        input = [
            '    BNE $00',
            '',
            'label:',
            '    BNE label'
        ],
        output = [
            0xd0, 0xfe, 0xd0, 0xfe
        ],
        nessemble = new Nessemble({
            stdin: input.join('\n'),
            onStdout: function (rc, stdout) {
                assert.deepEqual(stdout, output);
                done();
            }
        });

    nessemble.callMain();
});

/**
 * BPL
 */
QUnit.test('BPL', function (assert) {
    var done = assert.async(),
        input = [
            '    BPL $00',
            '',
            'label:',
            '    BPL label'
        ],
        output = [
            0x10, 0xfe, 0x10, 0xfe
        ],
        nessemble = new Nessemble({
            stdin: input.join('\n'),
            onStdout: function (rc, stdout) {
                assert.deepEqual(stdout, output);
                done();
            }
        });

    nessemble.callMain();
});

/**
 * BRK
 */
QUnit.test('BRK', function (assert) {
    var done = assert.async(),
        input = [
            '    BRK'
        ],
        output = [
            0x00
        ],
        nessemble = new Nessemble({
            stdin: input.join('\n'),
            onStdout: function (rc, stdout) {
                assert.deepEqual(stdout, output);
                done();
            }
        });

    nessemble.callMain();
});

/**
 * BVC
 */
QUnit.test('BVC', function (assert) {
    var done = assert.async(),
        input = [
            '    BVC $00',
            '',
            'label:',
            '    BVC label'
        ],
        output = [
            0x50, 0xfe, 0x50, 0xfe
        ],
        nessemble = new Nessemble({
            stdin: input.join('\n'),
            onStdout: function (rc, stdout) {
                assert.deepEqual(stdout, output);
                done();
            }
        });

    nessemble.callMain();
});

/**
 * BVS
 */
QUnit.test('BVS', function (assert) {
    var done = assert.async(),
        input = [
            '    BVS $00',
            '',
            'label:',
            '    BVS label'
        ],
        output = [
            0x70, 0xfe, 0x70, 0xfe
        ],
        nessemble = new Nessemble({
            stdin: input.join('\n'),
            onStdout: function (rc, stdout) {
                assert.deepEqual(stdout, output);
                done();
            }
        });

    nessemble.callMain();
});

/**
 * CLC
 */
QUnit.test('CLC', function (assert) {
    var done = assert.async(),
        input = [
            '    CLC'
        ],
        output = [
            0x18
        ],
        nessemble = new Nessemble({
            stdin: input.join('\n'),
            onStdout: function (rc, stdout) {
                assert.deepEqual(stdout, output);
                done();
            }
        });

    nessemble.callMain();
});

/**
 * CLD
 */
QUnit.test('CLD', function (assert) {
    var done = assert.async(),
        input = [
            '    CLD'
        ],
        output = [
            0xd8
        ],
        nessemble = new Nessemble({
            stdin: input.join('\n'),
            onStdout: function (rc, stdout) {
                assert.deepEqual(stdout, output);
                done();
            }
        });

    nessemble.callMain();
});

/**
 * CLI
 */
QUnit.test('CLI', function (assert) {
    var done = assert.async(),
        input = [
            '    CLI'
        ],
        output = [
            0x58
        ],
        nessemble = new Nessemble({
            stdin: input.join('\n'),
            onStdout: function (rc, stdout) {
                assert.deepEqual(stdout, output);
                done();
            }
        });

    nessemble.callMain();
});

/**
 * CLV
 */
QUnit.test('CLV', function (assert) {
    var done = assert.async(),
        input = [
            '    CLV'
        ],
        output = [
            0xb8
        ],
        nessemble = new Nessemble({
            stdin: input.join('\n'),
            onStdout: function (rc, stdout) {
                assert.deepEqual(stdout, output);
                done();
            }
        });

    nessemble.callMain();
});

/**
 * CMP
 */
QUnit.test('CMP', function (assert) {
    var done = assert.async(),
        input = [
            '    CMP #$44',
            '    CMP <$44',
            '    CMP <$44, X',
            '    CMP $4400',
            '    CMP $4400, X',
            '    CMP $4400, Y',
            '    CMP [$44, X]',
            '    CMP [$44], Y',
            '',
            'label:',
            '    CMP #label',
            '    CMP <label',
            '    CMP <label, X',
            '    CMP label',
            '    CMP label, X',
            '    CMP label, Y',
            '    CMP [label, X]',
            '    CMP [label], Y',
            '',
            'CONSTANT_8 = $44',
            'CONSTANT_16 = $4400',
            '',
            '    CMP #CONSTANT_8',
            '    CMP <CONSTANT_8',
            '    CMP <CONSTANT_8, X',
            '    CMP CONSTANT_16',
            '    CMP CONSTANT_16, X',
            '    CMP CONSTANT_16, Y',
            '    CMP [CONSTANT_8, X]',
            '    CMP [CONSTANT_8], Y'
        ],
        output = [
            0xc9, 0x44, 0xc5, 0x44, 0xd5, 0x44, 0xcd, 0x00, 0x44, 0xdd, 0x00, 0x44, 0xd9, 0x00, 0x44, 0xc1,
            0x44, 0xd1, 0x44, 0xc9, 0x13, 0xc5, 0x13, 0xd5, 0x13, 0xcd, 0x13, 0x00, 0xdd, 0x13, 0x00, 0xd9,
            0x13, 0x00, 0xc1, 0x13, 0xd1, 0x13, 0xc9, 0x44, 0xc5, 0x44, 0xd5, 0x44, 0xcd, 0x00, 0x44, 0xdd,
            0x00, 0x44, 0xd9, 0x00, 0x44, 0xc1, 0x44, 0xd1, 0x44
        ],
        nessemble = new Nessemble({
            stdin: input.join('\n'),
            onStdout: function (rc, stdout) {
                assert.deepEqual(stdout, output);
                done();
            }
        });

    nessemble.callMain();
});

/**
 * CPX
 */
QUnit.test('CPX', function (assert) {
    var done = assert.async(),
        input = [
            '    CPX #$44',
            '    CPX <$44',
            '    CPX $4400',
            '',
            'label:',
            '    CPX #label',
            '    CPX <label',
            '    CPX label',
            '',
            'CONSTANT_8 = $44',
            'CONSTANT_16 = $4400',
            '',
            '    CPX #CONSTANT_8',
            '    CPX <CONSTANT_8',
            '    CPX CONSTANT_16'
        ],
        output = [
            0xe0, 0x44, 0xe4, 0x44, 0xec, 0x00, 0x44, 0xe0, 0x07, 0xe4, 0x07, 0xec, 0x07, 0x00, 0xe0, 0x44,
            0xe4, 0x44, 0xec, 0x00, 0x44
        ],
        nessemble = new Nessemble({
            stdin: input.join('\n'),
            onStdout: function (rc, stdout) {
                assert.deepEqual(stdout, output);
                done();
            }
        });

    nessemble.callMain();
});

/**
 * CPY
 */
QUnit.test('CPY', function (assert) {
    var done = assert.async(),
        input = [
            '    CPY #$44',
            '    CPY <$44',
            '    CPY $4400',
            '',
            'label:',
            '    CPY #label',
            '    CPY <label',
            '    CPY label',
            '',
            'CONSTANT_8 = $44',
            'CONSTANT_16 = $4400',
            '',
            '    CPY #CONSTANT_8',
            '    CPY <CONSTANT_8',
            '    CPY CONSTANT_16'
        ],
        output = [
            0xc0, 0x44, 0xc4, 0x44, 0xcc, 0x00, 0x44, 0xc0, 0x07, 0xc4, 0x07, 0xcc, 0x07, 0x00, 0xc0, 0x44,
            0xc4, 0x44, 0xcc, 0x00, 0x44
        ],
        nessemble = new Nessemble({
            stdin: input.join('\n'),
            onStdout: function (rc, stdout) {
                assert.deepEqual(stdout, output);
                done();
            }
        });

    nessemble.callMain();
});
