# MQ2Cast

This is a plugin form of Spell_Routines.inc, and far, far superior. It can, among other things, memorize and cast spells, recast them, and, with the use of MQ2CastTimer, monitor the estimated duration on long term spells like DoTs, Buffs, etc. The plugin can also deal with events, similar to MQ2Events.

## Features

* Reliably casts spells (auto-recasts on fizzles or gate collapses). Will attempt to immobilize you before casting. Will cast immediately when a spell is available, before the gem is even fully ready.
* Intelligently and reliably memorizes spells and allows saving and loading of custom spell sets. Will attempt to immobilize you if moving.
* Clicks items and activates AAs. Will equip items in bags before clicking, and then return them once clicked.
* Utilizes MQ2Bandolier to swap in sets of items before casting (ie. Focus items).
* Auto-pause Stick and/or Advpath if detected and resume when complete.
* Custom interrupt function, to interrupt spells during casting (includes dismounting).
* Returns control to the macro immediately after it has started (ie. while the spell is casting), to allow the macro to perform other checks. Eg. while casting a heal spell, you could get the macro to check if the target has already been healed and then interrupt your spell if his HPs are above a certain amount.

## Commands

### /casting

`/casting "Name" [type] [options]`

Although the `[type]` is optional, it's always recommended to specify if you're casting a spell (specify the gem or number), clicking an item (specify "item" or the slot name/slot number) or activating an AA (specify "alt"), since there are AAs which have the same name as spells and spells and items can have the same IDs.

The `/casting` command requires only 1 argument, the name of the spell you wish to cast. If the spell is not memmed, it will try and mem it in gem5 (this is the default if no gem is specified). If the spell has more than one word, make sure to surround it with quotes. For example:  
`/casting "Minor Shielding"`  
This will cast the Minor Shielding spell if it is already memmed, or if not, will attempt to memorize it in gem5 before casting.

You can also include the gem number (or just the number) after the name of the spell, to indicate which gem it should be memmed into. For example:

```txt
/casting "Minor Shielding" 1
/casting "Minor Shielding" gem1
```

These will both cast Minor Shielding if memmed, or memorize it in gem1 before casting.

The gem number can also be appended to the end of the spell name, separated by a pipe command, like so:

```txt
/casting "Minor Shielding|gem1"
/casting "Minor Shielding|1"
```

You can also use the ID of the spell you wish to mem/cast. The following 4 examples all do the same thing:

```txt
/casting 288|gem1
/casting 288|1
/casting 288 gem1
/casting 288 1
```

All of the above will cast "Minor Shielding" (ID 288) and mem it in gem1 if not already memmed.

When clicking items you use the same syntax, except the "name" of the spell is the name of the item or the item's ID. It's always recommended to specify the type as "item" or specify the slot name/slot ID if you are clicking an item. For example:

```txt
/casting "Fabled Journeyman's Boots" item
/casting 68239|item
/casting "Shrunken Goblin Skull Earring" leftear
```

#### Options

The following optional parameters can be added to the /casting line:
* `bandolier|<name>` Equip the bandolier with `<name>` before casting. Useful for focus effects.
* `invis` With this parameter, it will not cast if you are invisible.
* `kill` Keep casting this spell until the target dies.
* `maxtries|<#>` Cast the spell this many times until successful.
* `recast|<#>` Recast the spell this many times.
* `setin|<name>` Same as bandolier (left here for backwards-compatibility purposes).
* `targetid|<#>` Target this ID before casting.

#### Examples

```txt
/casting "Wunshi's Focusing" gem4 -invis
/casting "Complete Heal|gem1" -kill -targetid|1234
/casting "Fizzle a Lot|gem4" -maxtries|5
/casting "uber nuke|9" -kill
/casting "cannibalize" gem3 -recast|5
```

### /interrupt

This will interrupt the current spell/item/AA that's casting.

### /memorize

`/memorize "name" [gem]`

As with the /casting command above, the /memorize command can take spell names or IDs, and can use gem# or just the numbers themselves. The gem# or number can also be appended to the end of the name.

See the following examples:

```txt
/memorize "Minor Shielding"
/memorize "Minor Shielding" 1
/memorize 288|gem1
```

Multiple spells can be memorized with the same line, such as the following example:  
`/memorize "1234|1" "gate|2" "cannibalize|gem4"`

As with /casting, the plugin will attempt to immobilize you before memorizing the spell.

### /sss

Spell set save, this will save your any number of your currently memmed spells into a spell set.

`/sss "name" [gems]`

Name is what you want your spell set to be called. If no gems are specified, all your gems are saved (ie. 123456789ABC). You can specify a smaller subset if needed.

```txt
/sss dps 1238
/sss dps 12ABC
/sss wunshi 5
```

### /ssm

Spell set memorize, this will memorize a previously saved spell set.

Example: `/ssm dps`

### /ssl

Spell Set List, this will list all spell sets that have been saved.

### /ssd

Spell Set Delete, this will delete a spell set from the ini file.

## TLOs

* bool `${Cast}` - Same as ${Cast.Active} (see below).
* bool `${Cast.Active}` - Return TRUE if plugin is loaded and you are in-game.
* spell `${Cast.Effect}` - Returns the name of the spell being casted, or a NULL string if not casting.
* bool `${Cast.Ready}` - Return TRUE if ready to cast a spell, item or AA.
  * bool `${Cast.Ready[M]}` - Return TRUE if ready to memorize a spell.
  * bool `${Cast.Ready[#]}` - Return TRUE if gem # is ready to cast.
  * bool `${Cast.Ready[X]}` - Return TRUE if spell, item, gem, ID, AA, etc is ready to cast.  As spells and items can have the same IDs and spells and AAs can have the same names, it's a good idea to specify the type of cast to take place (ie. gem#/item/alt). 
Examples:

```txt
${Cast.Ready[spellname or spellid]}
${Cast.Ready[aaname or aaid]}
${Cast.Ready[itemname or itemid]}
${Cast.Ready[1460]}
${Cast.Ready[Death Peace]}
${Cast.Ready[Death Peace|alt]}
${Cast.Ready[Death Peace|gem]}
${Cast.Ready[Death Peace|gem3]}
```

More help can be found here: https://www.redguides.com/community/resources/mq2cast.290/