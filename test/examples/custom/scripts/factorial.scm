(define (factorial n)
    (if (<= n 0)
        1
            (* n (factorial (- n 1)))))

(define custom
    (lambda (ints texts)
        (display
            (factorial
                (car ints)))))
