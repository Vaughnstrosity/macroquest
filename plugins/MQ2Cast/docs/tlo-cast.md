---
tags:
  - tlo
---
# `Cast`

<!--tlo-desc-start-->
Shows if plugin is loaded, and holds the Cast datatype which is much more interesting
<!--tlo-desc-end-->

## Forms
<!--tlo-forms-start-->
### {{ renderMember(type='bool', name='Cast') }}

:   Return TRUE if plugin is loaded and you are in-game.

<!--tlo-forms-end-->

## Associated DataTypes
<!--tlo-datatypes-start-->
## [`Cast`](datatype-cast.md)
{% include-markdown "projects/mq2cast/datatype-cast.md" start="<!--dt-desc-start-->" end="<!--dt-desc-end-->" trailing-newlines=false %} {{ readMore('projects/mq2cast/datatype-cast.md') }}
:    <h3>Members</h3>
    {% include-markdown "projects/mq2cast/datatype-cast.md" start="<!--dt-members-start-->" end="<!--dt-members-end-->" %}
    {% include-markdown "projects/mq2cast/datatype-cast.md" start="<!--dt-linkrefs-start-->" end="<!--dt-linkrefs-end-->" %}
    <!--tlo-datatypes-end-->

## Examples
<!--tlo-examples-start-->
```eqcommand
/echo ${Cast}
```
<!--tlo-examples-end-->

<!--tlo-linkrefs-start-->
[bool]: ../macroquest/reference/data-types/datatype-bool.md
<!--tlo-linkrefs-end-->