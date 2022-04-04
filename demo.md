```config
(set-title "Transmogrify User Manual") 
(set-author "Abhirag <hey@abhirag.com>")
(set-date "17 February 2022") 
(set-pwidth 500) 
(set-pheight 200)
```

```fe
(abstract (italic "Hello and welcome to the Transmogrify user manual. 
Transmogrify is a tool to convert markdown sprinkled with some lisp to LaTeX. This user manual 
has also been generated using Transmogrify, so feel free to refer to its source for reference"))
```

```fe
(toc "Configuration" "Lisp Extensions" "Pikchr")
```

# Configuration

Each document must start with a configuration code block:

`````
```config
; Set the title of the document
(set-title "Transmogrify User Manual")

; Set the author of the document 
(set-author "Abhirag <hey@abhirag.com>")

; Set the date of the document
(set-date "17 February 2022")

; Set the width of Pikchr figures 
(set-pwidth 1000)

; Set the height of Pikchr figures 
(set-pheight 500)
```
`````

# Lisp Extensions

We use an embedded lisp called _fe_ to extend the markdown syntax when necessary.

You write your document in plain markdown and use _fe_ code blocks for extra features

not supported by markdown such as:

## Abstract

`````
```fe
(abstract "text")
```
`````

## Table of Contents

`````
```fe
(toc "Configuration" "Lisp Extensions")
```
`````

## Sidenote

You can use inline _fe_ code to create a sidenote ``fe (sidenote "This is a sidenote" 0)``:

```
create a sidenote ``fe (sidenote "text" offset_in_mm)``
```

Sometimes a sidenote may run over the top of other text or graphics in the margin space.

If this happens, you can adjust the vertical position of the sidenote by providing a numerical offset argument.

## Marginnote

If you'd like to place ancillary information in the margin without the sidenote mark (the superscript number),

you can use a
marginnote ``fe (marginnote "This is a margin note. Notice that there isn't a number preceding the note, and there is no number in the main text where this note was written" 0)``:

```
create a marginnote ``fe (marginnote "text" offset_in_mm)``
```

## Text formatting in Lisp

You can format text written in lisp using three functions _italic_, _bold_ and _concat_.

Here is a formatted marginnote as an
example: ``fe (marginnote (concat (italic "This text is italic.") (bold "This text is bold")) 0)``

```
``fe (marginnote 
       (concat 
         (italic "This text is italic.") 
         (bold "This text is bold")) 0)``
```

# Pikchr

Embedded _Pikchr_ code will be rendered to a diagram:

`````
```pikchr
define ndblock {
  box wid boxwid/2 ht boxht/2
  down;  box same with .t at bottom of last box;   box same
}
boxht = .2; boxwid = .3; circlerad = .3; dx = 0.05
down; box; box; box; box ht 3*boxht "." "." "."
L: box; box; box invis wid 2*boxwid "hashtab:" with .e at 1st box .w
right
Start: box wid .5 with .sw at 1st box.ne + (.4,.2) "..."
N1: box wid .2 "n1";  D1: box wid .3 "d1"
N3: box wid .4 "n3";  D3: box wid .3 "d3"
box wid .4 "..."
N2: box wid .5 "n2";  D2: box wid .2 "d2"
arrow right from 2nd box
ndblock
spline -> right .2 from 3rd last box then to N1.sw + (dx,0)
spline -> right .3 from 2nd last box then to D1.sw + (dx,0)
arrow right from last box
ndblock
spline -> right .2 from 3rd last box to N2.sw-(dx,.2) to N2.sw+(dx,0)
spline -> right .3 from 2nd last box to D2.sw-(dx,.2) to D2.sw+(dx,0)
arrow right 2*linewid from L
ndblock
spline -> right .2 from 3rd last box to N3.sw + (dx,0)
spline -> right .3 from 2nd last box to D3.sw + (dx,0)
circlerad = .3
circle invis "ndblock"  at last box.e + (1.2,.2)
arrow dashed from last circle.w to 5/8<last circle.w,2nd last box> chop
box invis wid 2*boxwid "ndtable:" with .e at Start.w
```
`````

```pikchr
define ndblock {
  box wid boxwid/2 ht boxht/2
  down;  box same with .t at bottom of last box;   box same
}
boxht = .2; boxwid = .3; circlerad = .3; dx = 0.05
down; box; box; box; box ht 3*boxht "." "." "."
L: box; box; box invis wid 2*boxwid "hashtab:" with .e at 1st box .w
right
Start: box wid .5 with .sw at 1st box.ne + (.4,.2) "..."
N1: box wid .2 "n1";  D1: box wid .3 "d1"
N3: box wid .4 "n3";  D3: box wid .3 "d3"
box wid .4 "..."
N2: box wid .5 "n2";  D2: box wid .2 "d2"
arrow right from 2nd box
ndblock
spline -> right .2 from 3rd last box then to N1.sw + (dx,0)
spline -> right .3 from 2nd last box then to D1.sw + (dx,0)
arrow right from last box
ndblock
spline -> right .2 from 3rd last box to N2.sw-(dx,.2) to N2.sw+(dx,0)
spline -> right .3 from 2nd last box to D2.sw-(dx,.2) to D2.sw+(dx,0)
arrow right 2*linewid from L
ndblock
spline -> right .2 from 3rd last box to N3.sw + (dx,0)
spline -> right .3 from 2nd last box to D3.sw + (dx,0)
circlerad = .3
circle invis "ndblock"  at last box.e + (1.2,.2)
arrow dashed from last circle.w to 5/8<last circle.w,2nd last box> chop
box invis wid 2*boxwid "ndtable:" with .e at Start.w
```

