(define list
  (lambda args args))

(loop!
  (write-to! 'uart2
    (list 'pub
          (list
            (list 'uptime
                  (get-uptime!)))))
  (sleep 0.5))
