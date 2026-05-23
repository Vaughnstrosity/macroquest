---
tags:
  - plugin
resource_link: "https://www.redguides.com/community/resources/mq2cast.290/"
support_link: "https://www.redguides.com/community/threads/mq2cast.61518/"
repository: "https://github.com/RedGuides/MQ2Cast"
config: "Server_Character.ini"
authors: "s0rCieR, A_Enchanter_00, htw, devestator, Maskoi, EqMule, three-p-o, trev, alynel"
tagline: "A plugin to handle your character's casting, clicking and AA activation"
acknowledgements: "the authors of spell_routines.inc"
---

# MQ2Cast

<!--desc-start-->
This plugin handles the casting and recasting of spells, whether they're on your spell bar, on items, or even an AA. It also handles memorization and spell lists.
<!--desc-end-->
## Features
* Reliably casts spells (auto-recasts on fizzles or gate collapses). Will attempt to immobilize you before casting. Will cast immediately when a spell is available.
* Intelligently and reliably memorizes spells and allows saving and loading of custom spell sets. 
* Clicks items and activates AAs. Will equip items in bags before clicking, and then return them once clicked.
* Utilizes MQ2Bandolier to swap in sets of items before casting (ie. Focus items).
* Auto-pause Stick and/or Advpath if detected and resume when complete.
* Custom interrupt function, to interrupt spells during casting (includes dismounting).
* Returns control to the macro immediately after it has started (ie. while the spell is casting), to allow the macro to perform other checks. Eg. while casting a heal spell, you could get the macro to check if the target has already been healed and then interrupt your spell if his HPs are above a certain amount.

## Commands

<a href="cmd-castdebug/">
{% 
  include-markdown "projects/mq2cast/cmd-castdebug.md" 
  start="<!--cmd-syntax-start-->" 
  end="<!--cmd-syntax-end-->" 
%}
</a>
:    {% include-markdown "projects/mq2cast/cmd-castdebug.md" 
        start="<!--cmd-desc-start-->" 
        end="<!--cmd-desc-end-->" 
        trailing-newlines=false 
     %} {{ readMore('projects/mq2cast/cmd-castdebug.md') }}

<a href="cmd-casting/">
{% 
  include-markdown "projects/mq2cast/cmd-casting.md" 
  start="<!--cmd-syntax-start-->" 
  end="<!--cmd-syntax-end-->" 
%}
</a>
:    {% include-markdown "projects/mq2cast/cmd-casting.md" 
        start="<!--cmd-desc-start-->" 
        end="<!--cmd-desc-end-->" 
        trailing-newlines=false 
     %} {{ readMore('projects/mq2cast/cmd-casting.md') }}

<a href="cmd-interrupt/">
{% 
  include-markdown "projects/mq2cast/cmd-interrupt.md" 
  start="<!--cmd-syntax-start-->" 
  end="<!--cmd-syntax-end-->" 
%}
</a>
:    {% include-markdown "projects/mq2cast/cmd-interrupt.md" 
        start="<!--cmd-desc-start-->" 
        end="<!--cmd-desc-end-->" 
        trailing-newlines=false 
     %} {{ readMore('projects/mq2cast/cmd-interrupt.md') }}

<a href="cmd-memorize/">
{% 
  include-markdown "projects/mq2cast/cmd-memorize.md" 
  start="<!--cmd-syntax-start-->" 
  end="<!--cmd-syntax-end-->" 
%}
</a>
:    {% include-markdown "projects/mq2cast/cmd-memorize.md" 
        start="<!--cmd-desc-start-->" 
        end="<!--cmd-desc-end-->" 
        trailing-newlines=false 
     %} {{ readMore('projects/mq2cast/cmd-memorize.md') }}

<a href="cmd-ssd/">
{% 
  include-markdown "projects/mq2cast/cmd-ssd.md" 
  start="<!--cmd-syntax-start-->" 
  end="<!--cmd-syntax-end-->" 
%}
</a>
:    {% include-markdown "projects/mq2cast/cmd-ssd.md" 
        start="<!--cmd-desc-start-->" 
        end="<!--cmd-desc-end-->" 
        trailing-newlines=false 
     %} {{ readMore('projects/mq2cast/cmd-ssd.md') }}

<a href="cmd-ssl/">
{% 
  include-markdown "projects/mq2cast/cmd-ssl.md" 
  start="<!--cmd-syntax-start-->" 
  end="<!--cmd-syntax-end-->" 
%}
</a>
:    {% include-markdown "projects/mq2cast/cmd-ssl.md" 
        start="<!--cmd-desc-start-->" 
        end="<!--cmd-desc-end-->" 
        trailing-newlines=false 
     %} {{ readMore('projects/mq2cast/cmd-ssl.md') }}

<a href="cmd-ssm/">
{% 
  include-markdown "projects/mq2cast/cmd-ssm.md" 
  start="<!--cmd-syntax-start-->" 
  end="<!--cmd-syntax-end-->" 
%}
</a>
:    {% include-markdown "projects/mq2cast/cmd-ssm.md" 
        start="<!--cmd-desc-start-->" 
        end="<!--cmd-desc-end-->" 
        trailing-newlines=false 
     %} {{ readMore('projects/mq2cast/cmd-ssm.md') }}

<a href="cmd-sss/">
{% 
  include-markdown "projects/mq2cast/cmd-sss.md" 
  start="<!--cmd-syntax-start-->" 
  end="<!--cmd-syntax-end-->" 
%}
</a>
:    {% include-markdown "projects/mq2cast/cmd-sss.md" 
        start="<!--cmd-desc-start-->" 
        end="<!--cmd-desc-end-->" 
        trailing-newlines=false 
     %} {{ readMore('projects/mq2cast/cmd-sss.md') }}

## Settings

The configuration file is in the `server_character.ini` file, and holds spells sets for memorization.

Example for a spellset called "gandalfDPS"

```ini
[MQ2Cast(SpellSet)]
gandalfDPS 12=26|1 200|2
```

- "gandalfDPS" is the spell set name, 
- "12=" refers to spell gems 1 and 2
- "26|1" is spellID **26**, spell gem **1**

These are easier to configure via commands than direct editing of the .ini

## See also

- [/cast](../macroquest/reference/commands/cast.md)

## Top-Level Objects

## [Cast](tlo-cast.md)
{% include-markdown "projects/mq2cast/tlo-cast.md" start="<!--tlo-desc-start-->" end="<!--tlo-desc-end-->" trailing-newlines=false %} {{ readMore('projects/mq2cast/tlo-cast.md') }}

## DataTypes

## [Cast](datatype-cast.md)
{% include-markdown "projects/mq2cast/datatype-cast.md" start="<!--dt-desc-start-->" end="<!--dt-desc-end-->" trailing-newlines=false %} {{ readMore('projects/mq2cast/datatype-cast.md') }}

<h2>Members</h2>
{% include-markdown "projects/mq2cast/datatype-cast.md" start="<!--dt-members-start-->" end="<!--dt-members-end-->" %}
{% include-markdown "projects/mq2cast/datatype-cast.md" start="<!--dt-linkrefs-start-->" end="<!--dt-linkrefs-end-->" %}
