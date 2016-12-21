/**
 * Exponent math function
 * @param {float} x - Base
 * @param {int} y - Power
 */
int power(int x, int y) {
    int temp = 0;

    if (y == 0) {
        return 1;
    }

    temp = power(x, (int)(y / 2));

    if ((y % 2) == 0) {
        return temp * temp;
    } else {
        if (y > 0) {
            return x * temp * temp;
        } else {
            return (int)((temp * temp) / x);
        }
    }
}
