/* Example:
    * size x 10, pos x 5
    * print 10 chars, pos x 15
    * term.pos.x = 5 (15 % 10 = 5)
*/

/* Example:
     * size x 10, pos x 1, size y 10, pos y 0
     * prints 8 characters, pos x 9 (pos x + printed = 9)
     * Formula: (new pos x % size x = new pos x)
     * term.pos.x = 5 (9 % 10 = 0.something, size_t 0.8 or 0.8 = 1)
     * this needs a separate case, or y will be incorrect
*/

/* Example:
     * size x 10, pos x 5, size y 10, pos y 1
     * print 25 chars, pos x 30 (pos x + printed = 30)
     * Formula: (new pos x / size x = float coerced to int/size_t)
     * term.pos.y = (30 / 10 = 3, (size_t)3 = 3)
*/

/* Example:
     * size x 10, pos x 5, size y 10, pos y 0
     * print 30 chars, pos x 35 (pos x + printed = 35)
     * Formula: (new pos x / size x = float coerced to int/size_t)
     * term.pos.y = (35 / 10 = 3.5, (size_t)3.5 = 3)
*/

/* Example:
    * size x 10, pos x 5, size y 10, pos y 0
    * print 10 chars, pos x 15
    * Formula: (new pos x % size x = new pos x)
    * term.pos.x = 5 (15 % 10 = 5)
    * Formula: (new pos x / size x = float coerced to int/size_t)
    * term.pos.y = 1 (15 / 10 = 1.5, (size_t)1.5 = 1)
*/
