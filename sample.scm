(define sum (lambda (xs)
    (apply + xs)))

(define average (lambda (xs)
    (/ (sum xs) (length xs))))

