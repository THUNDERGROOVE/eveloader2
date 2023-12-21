# Python Console Snippets

This is a list of python snippets that can be pasted into the python console while using eveloader2 and may be helpful for development and bug testing.  The python console will only open when the use_console configuration option is set to true.  Please do not use the python console and these scripts servers that are not yours without permission.  

## give all skills

Give the current character every skill possible at level 5.  This will take some time to complete.  This works by sending the command /giveskill for every skill in the game.  It has a built in delay in the loop to not strain the server.

``` python
skills = sm.GetService("skills").GetAllSkills()
for skill in skills:
    blue.pyos.synchro.SleepSim(100)
    eve.Message('CustomNotify', {'notify': 'Giving skill {}'.format(cfg.invtypes.Get(skill.typeID))})
    sm.GetService('slash').SlashCmd('/giveskill me {} 5'.format(skill.typeID))

```

## Toggle extrapolation of objects in space

``` python
sm.GetService('movementClient').ToggleExtrapolation()
```
