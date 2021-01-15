;;;
;;;  Copyright (C) 2016 Stephane D'Alu
;;;
;;;  Licensed under the Apache License, Version 2.0 (the "License");
;;;  you may not use this file except in compliance with the License.
;;;  You may obtain a copy of the License at
;;;
;;;      http://www.apache.org/licenses/LICENSE-2.0
;;;
;;;  Unless required by applicable law or agreed to in writing, software
;;;  distributed under the License is distributed on an "AS IS" BASIS,
;;;  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
;;;  See the License for the specific language governing permissions and
;;;  limitations under the License.

;;;
;;; See: https://www.emacswiki.org/emacs/IndentingC
;;;

;;
;;; Loading of this file can be done in ~/.emacs
;;; by putting: (load "~/path/to/ChibiOS-Contrib/tools/chibios.el")
;;;
;;; Styling can be selected using local variable:
;;;   /* -*- c-file-style: "chibios" -*- */
;;;
;;; But will also be automatically apply to a file located in a
;;; ChibiOS directory. Example:  /path/to/../ChibiOS/../file.c
;;;



;;
;; Define ChibiOS prefered styling
;;
(defconst chibios-c-style
  '((indent-tabs-mode . nil)
    (c-basic-offset   . 2))
  "ChibiOS C Programming Style")

(c-add-style "chibios" chibios-c-style)


;;
;; 
;;
(defun maybe-chibios-c-style ()
  (when (and buffer-file-name
	     (string-match "ChibiOS" buffer-file-name))
    (c-set-style "chibios")))

(add-hook 'c-mode-hook 'maybe-chibios-c-style)

