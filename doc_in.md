```config
(set-title "Documenting Scribe using itself") 
(set-author "Abhirag <hey@abhirag.com>")
(set-date "22 February 2022") 
(set-pwidth 500) 
(set-pheight 200)
```

```fe
(abstract "Reading and comprehending a codebase is such a time taking endeavor because even in well 
commented codebases all we have at our disposal is a grab bag of facts without a narrative tying them. 
The structure of our programs is still based on the whims of the compiler/interpreter and exploring 
codebases is still a very rigid and manual process.

Scribe is a tool based on the premise that code is data which aims to alleviate some of the problems 
mentioned above by exposing that data to the programmers for documentation and exploration.")
```

```scribe
(let [db-c (core/file-src "./src" "db.c")]
  (c/function-definition "db_get" db-c))
```
