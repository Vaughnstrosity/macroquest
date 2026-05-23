---
tags:
  - datatype
---
# `Cast`

<!--dt-desc-start-->
Returns information about plugin status, current spell being cast, spell readiness and more.
<!--dt-desc-end-->

## Members
<!--dt-members-start-->
### {{ renderMember(type='bool', name='Active') }}

:   Return TRUE if plugin is loaded and you are in-game.

### {{ renderMember(type='spell', name='Effect') }}

:   Returns the name of the spell being cast, or a NULL string if not casting.

### {{ renderMember(type='bool', name='Ready', params='X') }}

:   Returns TRUE if ready to cast based on the parameter provided:

    **No parameter:**
    - `${Cast.Ready}` - Returns TRUE if ready to cast a spell, item or AA

    **Special parameters:**
    - `${Cast.Ready[M]}` - Returns TRUE if ready to memorize a spell
    - `${Cast.Ready[#]}` - Returns TRUE if gem # is ready to cast (where # is a gem number)

    **Name or ID parameters:**
    - `${Cast.Ready[<name or ID>]}` - Returns TRUE if the specified spell, item, gem, ID, AA, etc. is ready to cast
    
    !!! note "Disambiguation"
        Since spells and items can have the same IDs, and spells and AAs can have the same names, it's recommended to specify the type of cast (e.g., `gem#`, `item`, `alt`). See examples below for usage patterns.

### {{ renderMember(type='string', name='Result') }}

:   Returns a string containing the result of the `/casting` command.

    **Possible result values:**
    
    | Result | Description |
    |--------|-------------|
    | `CAST_ABORTED` | Casting Aborted (`/interrupt`) |
    | `CAST_CANCELLED` | Casting was aborted |
    | `CAST_CANNOTSEE` | Cannot see target |
    | `CAST_COLLAPSE` | Your Gate collapsed |
    | `CAST_COMPONENTS` | Missing Component |
    | `CAST_DISTRACTED` | You were distracted |
    | `CAST_FIZZLE` | Your cast fizzled |
    | `CAST_IMMUNE` | Target is immune the spell's effect |
    | `CAST_INTERRUPTED` | Casting was interrupted |
    | `CAST_INVISIBLE` | You are invisible |
    | `CAST_NOTARGET` | No target |
    | `CAST_NOTREADY` | Not ready to cast |
    | `CAST_OUTOFMANA` | Not enough mana to cast spell |
    | `CAST_OUTOFRANGE` | Target is out of range |
    | `CAST_OUTDOORS` | Spell not working here (on mount etc.) |
    | `CAST_PENDING` | Casting is in progress |
    | `CAST_RECOVER` | Spell is not ready |
    | `CAST_RESIST` | Cast was resisted |
    | `CAST_STANDING` | Not standing |
    | `CAST_STUNNED` | You are stunned |
    | `CAST_SUCCESS` | The cast was a success |
    | `CAST_TAKEHOLD` | The spell did not take hold |
    | `CAST_UNKNOWN` | Unknown Spell |

### {{ renderMember(type='string', name='Return') }}

:   Returns the result of the casting/memorize/interrupt request.

### {{ renderMember(type='string', name='Status') }}

:   Returns a string containing all the pending events. This string often contains multiple events (e.g., when `/casting` still has to immobilize you and then memorize the spell before it can cast).

    **Possible pending event codes:**
    
    | Code | Description |
    |------|-------------|
    | **I** | Idle and waiting for you |
    | **A** | Advpath pause |
    | **F** | Stick pause |
    | **S** | Immobilize in progress |
    | **M** | Memorize in progress |
    | **E** | Item swapped |
    | **D** | Ducking casting |
    | **T** | Targeting |
    | **C** | Spell casting in progress |

### {{ renderMember(type='spell', name='Stored') }}

:   Returns the last spell that was cast, or NULL if no spell has been cast.

### {{ renderMember(type='bool', name='Taken') }}

:   Return TRUE if last spell cast didn't take hold on target.

### {{ renderMember(type='int', name='Timing') }}

:   Returns the estimated number of miliseconds remaining until the spell finished casting.

<!--dt-members-end-->

## Examples
<!--dt-examples-start-->
=== "MQScript"

    ```text
    | Check if a spell is ready
    /echo ${Cast.Ready[Death Peace]}

    | Check if an AA is ready
    /echo ${Cast.Ready[Death Peace|alt]}

    | Check if an item is ready by name
    /echo ${Cast.Ready[Mod Rod]}

    | Check by spell / item ID
    /echo ${Cast.Ready[1460]}

    | Check if gem 3 is ready
    /echo ${Cast.Ready[gem3]}
    ```

=== "Lua"

    ```lua
    -- Check if a spell is ready
    print(mq.TLO.Cast.Ready("Death Peace")())

    -- Check if an AA is ready
    print(mq.TLO.Cast.Ready("Death Peace|alt")())

    -- Check if an item is ready by name
    print(mq.TLO.Cast.Ready("Mod Rod")())

    -- Check by spell / item ID
    print(mq.TLO.Cast.Ready("1460")())

    -- Check if gem 3 is ready
    print(mq.TLO.Cast.Ready("gem3")())
    ```
<!--dt-examples-end-->

<!--dt-linkrefs-start-->
[bool]: ../macroquest/reference/data-types/datatype-bool.md
[int]: ../macroquest/reference/data-types/datatype-int.md
[spell]: ../macroquest/reference/data-types/datatype-spell.md
[string]: ../macroquest/reference/data-types/datatype-string.md
<!--dt-linkrefs-end-->
