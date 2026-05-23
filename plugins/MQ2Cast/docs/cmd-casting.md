---
tags:
  - command
---

# /casting

## Syntax

<!--cmd-syntax-start-->
```eqcommand
/casting "<name>" [type] [option]...
```
<!--cmd-syntax-end-->

## Description

<!--cmd-desc-start-->
The /casting command requires only 1 argument, the name of the spell you wish to cast. If the spell is not memmed, it will try and mem it in gem5 by default. If the spell has more than one word, make sure to surround it with quotes. 
<!--cmd-desc-end-->

!!! note "Use type to avoid same-name/ID conflicts"
    Although the 'type' is optional, it's always recommended to specify if you're casting a spell (specify the gem or number), clicking an item (specify "item" or the slot name/slot number) or activating an AA (specify "alt"), since there are AAs which have the same name as spells, and spells and items can have the same IDs.

## Options

| Option | Description |
|--------|-------------|
| `<"name">` | Required. This can be the name of a spell, the spell ID, the name of an item (the item itself, not the spell attached to it), the item ID, an AA, or the AA ID. |
| `[type]` | Not required but heavily recommended due to naming conflicts between AA's, spells and items. See Types table below for available values. |
| `bandolier|<name>` | Equip the bandolier with `<name>` before casting. Useful for focus effects. |
| `invis` | With this parameter, it will not cast if you are invisible. |
| `kill` | Keep casting until the target dies. |
| `maxtries|<#>` | Cast the spell through this many failures until successful. |
| `recast|<#>` | Recast the spell this many times. |
| `setin|<name>` | Same as bandolier (left here for backwards-compatibility purposes). |
| `targetid|<#>` | Target this ID before casting. |

### Types

| Type | Description |
|------|-------------|
| `#` or `gem#` | The number of the spell gem this will be memmed to if not currently memorized. This also clarifies that you're casting a spell. |
| `item` or `slotname` | Clarifies that you're casting an item. You can use "item" or specify the actual slot name. |
| `alt` | Clarifies that you're casting an AA. |

### Basic Spell Casting

Cast a spell if it's already memorized, or memorize it in gem5 before casting:
```eqcommand
/casting "Minor Shielding"
```

### Specifying Spell Gems

Cast a spell and specify which gem to memorize it in (all equivalent):
```eqcommand
/casting "Minor Shielding" 1
/casting "Minor Shielding" gem1
/casting "Minor Shielding|gem1"
/casting "Minor Shielding|1"
```

### Using Spell IDs

Cast using spell ID (Minor Shielding ID 288) and memorize in gem1:
```eqcommand
/casting 288|gem1
/casting 288|1
/casting 288 gem1
/casting 288 1
```

### Item Casting

Click items to activate their effects:
```eqcommand
/casting "Fabled Journeyman's Boots" item
/casting 68239|item
/casting "Shrunken Goblin Skull Earring" leftear
```

### AA Casting

Activate Alternate Abilities:
```eqcommand
/casting "Death Peace" alt
```

### Advanced Options

Examples using various command options:
```eqcommand
/casting "Wunshi's Focusing" gem4 -invis
/casting "Complete Heal|gem1" -kill -targetid|1234
/casting "Fizzle a Lot|gem4" -maxtries|5
/casting "uber nuke|9" -kill
/casting "cannibalize" gem3 -recast|5
```
