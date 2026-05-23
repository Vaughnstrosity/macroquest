---
tags:
  - command
---

# /memorize

## Syntax

<!--cmd-syntax-start-->
```eqcommand
/memorize <name> [gem]
```
<!--cmd-syntax-end-->

## Description

<!--cmd-desc-start-->
Take spell names or IDs, and can use gem# or just the numbers themselves. The gem# or number can also be appended to the end of the name. e.g.
```eqcommand
/memorize "Minor Shielding" 1
```
The plugin will attempt to immobilize you before memorizing the spell(s).
<!--cmd-desc-end-->

## Examples

- As with the /casting command above, the /memorize command can take spell names or IDs, and can use gem# or just the numbers themselves. The gem# or number can also be appended to the end of the name.
See the following examples:
```eqcommand
/memorize "Minor Shielding"
/memorize "Minor Shielding" 1
/memorize 288|gem1
```
- Multiple spells can be memorized with the same line, such as the following example:
```eqcommand
/memorize "1234|1" "gate|2" "cannibalize|gem4"
```
