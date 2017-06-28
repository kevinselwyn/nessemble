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
