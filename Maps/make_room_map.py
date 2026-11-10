"""Generate CdmRoom.World00a: two grid rooms with a doorway, two crates, lights, a start point.
Format = text World00a as written by WorldEdit (see research/123.decoded.World00a).
usage: python -I make_room_map.py <out.World00a>"""
import struct, sys

def hx(f):
    return '0x%08X' % struct.unpack('<I', struct.pack('<f', float(f)))

FLOOR, WALL, CEIL = r'global\materials\grid_floor.Mat00', r'global\materials\grid_wall.Mat00', r'global\materials\grid_ceiling.Mat00'
S = 0.01

def textureinfo(face):
    # face: 'y' (floor/ceiling), 'x', 'z'
    if face == 'y':
        p, q = (S, 0, 0), (0, 0, -S)
    elif face == 'x':
        p, q = (0, 0, S), (0, -S, 0)
    else:
        p, q = (S, 0, 0), (0, -S, 0)
    o = '( %s %s %s )' % (hx(0), hx(0), hx(0))
    return '( textureinfo\n\t\t\t\t\t\t\t\t%s\n\t\t\t\t\t\t\t\t( %s %s %s )\n\t\t\t\t\t\t\t\t( %s %s %s )\n\t\t\t\t\t\t\t)' % (o, hx(p[0]), hx(p[1]), hx(p[2]), hx(q[0]), hx(q[1]), hx(q[2]))

# box topology of the editor export: points 0..3 top (y1), 4..7 bottom (y0)
FACES = [((0, 1, 2, 3), 'y', 'top'), ((4, 5, 1, 0), 'x', 'west'), ((5, 6, 2, 1), 'z', 'north'), ((6, 7, 3, 2), 'x', 'east'), ((7, 4, 0, 3), 'z', 'south'), ((7, 6, 5, 4), 'y', 'bottom')]

def polyhedron(box, material):
    x0, x1, y0, y1, z0, z1 = box
    pts = [(x0, y1, z0), (x0, y1, z1), (x1, y1, z1), (x1, y1, z0), (x0, y0, z0), (x0, y0, z1), (x1, y0, z1), (x1, y0, z0)]
    out = ['\t\t( polyhedron (', '\t\t\t( color 255 255 255 )', '\t\t\t( pointlist ']
    for p in pts:
        out.append('\t\t\t\t( %s %s %s 255 255 255 255 )' % (hx(p[0]), hx(p[1]), hx(p[2])))
    out += ['\t\t\t)', '\t\t\t( polylist (']
    for idx, axis, _ in FACES:
        mat = material if material else ''
        out.append('\t\t\t\t( editpoly \n\t\t\t\t\t( f %d %d %d %d )\n\t\t\t\t\t( material "%s" )\n\t\t\t\t\t( occlusion "" )\n\t\t\t\t\t( mappings (' % (*idx, mat))
        for m in range(4):
            out.append('\t\t\t\t\t\t( %d\n\t\t\t\t\t\t\t%s\n\t\t\t\t\t\t) ' % (m, textureinfo(axis) if mat else '( textureinfo\n\t\t\t\t\t\t\t\t( 0x00000000 0x00000000 0x00000000 )\n\t\t\t\t\t\t\t\t( 0x00000000 0x00000000 0x00000000 )\n\t\t\t\t\t\t\t\t( 0x00000000 0x00000000 0x00000000 )\n\t\t\t\t\t\t\t)'))
        out.append('\t\t\t\t\t) ) \n\t\t\t\t) ')
    out += ['\t\t\t) )', '\t\t) )']
    return '\n'.join(out)

# (name, box, material, type)
H = 300
W = 32
solid = [
    ('Floor',      (-532, 532, -W, 0, -532, 532), FLOOR),
    ('Ceiling',    (-532, 532, H, H + W, -532, 532), CEIL),
    ('WallWest',   (-532, -500, 0, H, -532, 532), WALL),
    ('WallEast',   (500, 532, 0, H, -532, 532), WALL),
    ('WallSouth',  (-500, 500, 0, H, -532, -500), WALL),
    ('WallNorth',  (-500, 500, 0, H, 500, 532), WALL),
    ('DivLeft',    (-500, -64, 0, H, -16, 16), WALL),
    ('DivRight',   (64, 500, 0, H, -16, 16), WALL),
    ('DivLintel',  (-64, 64, 220, H, -16, 16), WALL),
    ('CrateA',     (-300, -220, 0, 80, -350, -270), WALL),
    ('CrateB',     (200, 300, 0, 100, 250, 350), WALL),
]
EXTRA_SECTORS = []  # additional Sector brushes (name, box); used by make_stress_map.py
SECTOR = ('Sector', (-500, 500, 0, H, -500, 500), '')

BRUSH_PROPS = '''\t\t( proplist (
\t\t\t(  string "Name" (  ) ( data "%s") )
\t\t\t(  string "Type" ( staticlist  ) ( data "%s") )
\t\t\t(  bool "NotAStep" (  ) ( data 0 ))
\t\t\t(  bool "ClipLight" (  ) ( data 1 ))
\t\t\t(  real "CreaseAngle" (  ) ( data 45.000000 ))
\t\t\t(  string "TangentMethod" ( staticlist  ) ( data "World") )
\t\t\t(  string "ShadowLOD" ( staticlist  ) ( data "Low") )
\t\t\t(  vector "Pos" ( hidden distance  ) ( data ( vector (0.000000 0.000000 0.000000) ) ) )
\t\t\t(  rotation "Rotation" ( hidden  ) ( data (eulerangles (0.000000 0.000000 0.000000) ) ) )
\t\t) )'''

START_PROPS = '''\t\t( proplist (
\t\t\t(  string "Name" (  ) ( data "GameStartPoint00") )
\t\t\t(  vector "Pos" ( distance  ) ( data ( vector (0.000000 60.000000 -300.000000) ) ) )
\t\t\t(  rotation "Rotation" (  ) ( data (eulerangles (0.000000 0.000000 0.000000) ) ) )
\t\t\t(  bool "Template" (  ) ( data 0 ))
\t\t\t(  bool "IsGore" (  ) ( data 0 ))
\t\t\t(  string "ObjectLOD" ( staticlist  ) ( data "Low") )
\t\t\t(  string "ModelTemplate" ( staticlist  ) ( data "M01_Player") )
\t\t\t(  string "PhysicsModel" ( staticlist  ) ( data "Normal") )
\t\t\t(  command "Command" ( notifychange  ) ( data ""))
\t\t\t(  bool "Locked" (  ) ( data 0 ))
\t\t\t(  bool "SendCommandOnRespawn" (  ) ( data 0 ))
\t\t\t(  string "Team" ( staticlist  ) ( data "NoTeam") )
\t\t\t(  bool "StartPoint" (  ) ( data 1 ))
\t\t\t(  bool "SpawnPoint" (  ) ( data 1 ))
\t\t) )'''

LIGHT_PROPS = '''\t\t( proplist (
\t\t\t(  string "Name" (  ) ( data "%s") )
\t\t\t(  vector "Pos" ( distance  ) ( data ( vector (%s) ) ) )
\t\t\t(  rotation "Rotation" (  ) ( data (eulerangles (0.000000 0.000000 0.000000) ) ) )
\t\t\t(  bool "Template" (  ) ( data 0 ))
\t\t\t(  bool "IsGore" (  ) ( data 0 ))
\t\t\t(  string "ObjectLOD" ( staticlist  ) ( data "Low") )
\t\t\t(  bool "ShadowDependent" (  ) ( data 0 ))
\t\t\t(  string "LightLOD" ( staticlist  ) ( data "Low") )
\t\t\t(  string "WorldShadowsLOD" ( staticlist  ) ( data "Low") )
\t\t\t(  string "ObjectShadowsLOD" ( staticlist  ) ( data "Low") )
\t\t\t(  real "LightRadius" ( radius distance  ) ( data 700.000000 ))
\t\t\t(  real "FadeStartDist" ( radius distance  ) ( data 0.000000 ))
\t\t\t(  real "FadeEndDist" ( radius distance  ) ( data 0.000000 ))
\t\t\t(  color "LightColor" (  ) ( data ( vector (255.000000 240.000000 220.000000) ) ) )
\t\t\t(  color "TranslucentColor" (  ) ( data ( vector (255.000000 255.000000 255.000000) ) ) )
\t\t\t(  color "SpecularColor" (  ) ( data ( vector (255.000000 255.000000 255.000000) ) ) )
\t\t\t(  real "IntensityScale" (  ) ( data 1.000000 ))
\t\t) )'''

def quad_polyhedron(x0, x1, y, z0, z1):
    """Single upward polygon (the AINavMesh surface)."""
    pts = [(x0, y, z0), (x0, y, z1), (x1, y, z1), (x1, y, z0)]
    out = ['\t\t( polyhedron (', '\t\t\t( color 255 255 255 )', '\t\t\t( pointlist ']
    for p in pts:
        out.append('\t\t\t\t( %s %s %s 255 255 255 255 )' % (hx(p[0]), hx(p[1]), hx(p[2])))
    out += ['\t\t\t)', '\t\t\t( polylist (', '\t\t\t\t( editpoly \n\t\t\t\t\t( f 0 1 2 3 )\n\t\t\t\t\t( material "" )\n\t\t\t\t\t( occlusion "" )\n\t\t\t\t\t( mappings (']
    for m in range(4):
        out.append('\t\t\t\t\t\t( %d\n\t\t\t\t\t\t\t( textureinfo\n\t\t\t\t\t\t\t\t( 0x00000000 0x00000000 0x00000000 )\n\t\t\t\t\t\t\t\t( 0x00000000 0x00000000 0x00000000 )\n\t\t\t\t\t\t\t\t( 0x00000000 0x00000000 0x00000000 )\n\t\t\t\t\t\t\t)\n\t\t\t\t\t\t) ' % m)
    out += ['\t\t\t\t\t) ) \n\t\t\t\t) ', '\t\t\t) )', '\t\t) )']
    return '\n'.join(out)

NAV_PROPS = '''\t\t( proplist (
\t\t\t(  string "Name" (  ) ( data "CdmRoomNav") )
\t\t\t(  vector "Pos" ( distance  ) ( data ( vector (0.000000 0.000000 0.000000) ) ) )
\t\t\t(  rotation "Rotation" (  ) ( data (eulerangles (0.000000 0.000000 0.000000) ) ) )
\t\t\t(  bool "Template" (  ) ( data 0 ))
\t\t\t(  bool "IsGore" (  ) ( data 0 ))
\t\t\t(  string "ObjectLOD" ( staticlist  ) ( data "Low") )
\t\t\t(  string "NavMeshType" ( staticlist  ) ( data "AI") )
\t\t\t(  string "MaskType" ( staticlist  ) ( data "Include") )
\t\t\t(  real "BlindDataIndex" ( hidden  ) ( data -1.000000 ))
\t\t\t(  longint "AINavMeshIndex" ( hidden  ) ( data -1.000000 ))
''' + ''.join('\t\t\t(  string "CharacterType%d" ( staticlist  ) ( data "None") ) \n' % i for i in range(16)) + '\t\t) )'

STALK_PROPS = '''\t\t( proplist (
\t\t\t(  string "Name" (  ) ( data "AINodeStalk00") )
\t\t\t(  vector "Pos" ( distance  ) ( data ( vector (0.000000 0.000000 0.000000) ) ) )
\t\t\t(  rotation "Rotation" (  ) ( data (eulerangles (0.000000 0.000000 0.000000) ) ) )
\t\t\t(  bool "Template" (  ) ( data 0 ))
\t\t\t(  bool "IsGore" (  ) ( data 0 ))
\t\t\t(  string "ObjectLOD" ( staticlist  ) ( data "Low") )
\t\t\t(  bool "Face" (  ) ( data 1 ))
\t\t\t(  rotation "FacingOffset" ( relativeorientation ) ( data (eulerangles (0.000000 0.000000 0.000000) ) ) )
\t\t\t(  string "CharacterType" ( staticlist  ) ( data "None") )
\t\t\t(  bool "StartDisabled" (  ) ( data 0 ))
\t\t\t(  string "Object" ( objectlink  ) ( data "") )
\t\t\t(  string "Dependency" ( objectlink  ) ( data "") )
\t\t\t(  real "Radius" ( radius  ) ( data 512.000000 ))
\t\t\t(  string "Region" ( objectlink  ) ( data "") )
\t\t\t(  command "PreActivateCommand" ( notifychange  ) ( data ""))
\t\t\t(  command "PostActivateCommand" ( notifychange  ) ( data ""))
\t\t\t(  real "ReactivationTime" (  ) ( data 0.000000 ))
\t\t\t(  string "FirstSound" ( staticlist  ) ( data "None") )
\t\t\t(  string "FidgetSound" ( staticlist  ) ( data "None") )
\t\t\t(  string "SmartObject" ( dims staticlist  ) ( data "None") )
\t\t\t(  real "BoundaryRadius" ( radius fovfarz  ) ( data 1024.000000 ))
\t\t\t(  string "BoundaryRegion" ( objectlink  ) ( data "") )
\t\t\t(  string "ModelName" ( hidden model  ) ( data "") )
\t\t\t(  real "StalkDataIndex" ( hidden  ) ( data -1.000000 ))
\t\t) )'''

CAI_PROPS = '''\t\t( proplist (
\t\t\t(  string "Name" (  ) ( data "CdmAddict00") )
\t\t\t(  vector "Pos" ( distance  ) ( data ( vector (0.000000 60.000000 250.000000) ) ) )
\t\t\t(  rotation "Rotation" (  ) ( data (eulerangles (0.000000 3.141593 0.000000) ) ) )
\t\t\t(  bool "Template" (  ) ( data 0 ))
\t\t\t(  bool "IsGore" (  ) ( data 0 ))
\t\t\t(  string "ObjectLOD" ( staticlist  ) ( data "Low") )
\t\t\t(  bool "MoveToFloor" (  ) ( data 1 ))
\t\t\t(  bool "MakeBody" (  ) ( data 1 ))
\t\t\t(  bool "PermanentBody" (  ) ( data 0 ))
\t\t\t(  real "BodyLifetime" (  ) ( data -1.000000 ))
\t\t\t(  string "ModelTemplate" ( staticlist  ) ( data "M01_Addict3ONESHOT") )
\t\t\t(  string "Brain" ( staticlist  ) ( data "Default") )
\t\t\t(  string "GoalSet" ( staticlist  ) ( data "Patrol") )
\t\t\t(  bool "CanTalk" (  ) ( data 1 ))
\t\t\t(  real "Mass" (  ) ( data 30.000000 ))
\t\t\t(  bool "CanHeal" (  ) ( data 1 ))
\t\t\t(  bool "CanRepair" (  ) ( data 1 ))
\t\t\t(  bool "CanDamage" (  ) ( data 1 ))
\t\t\t(  bool "NeverDestroy" (  ) ( data 0 ))
\t\t\t(  bool "UseDefaultAttachments" (  ) ( data 1 ))
\t\t\t(  bool "UseDefaultWeapons" (  ) ( data 1 ))
\t\t\t(  bool "HolsterWeapons" (  ) ( data 0 ))
\t\t\t(  string "Alignment" ( staticlist  ) ( data "Default") )
\t\t\t(  string "Awareness" ( staticlist  ) ( data "Relaxed") )
\t\t\t(  string "AwarenessMod" ( staticlist  ) ( data "None") )
\t\t\t(  bool "SeekEnemy" (  ) ( data 0 ))
\t\t\t(  real "HitPoints" (  ) ( data -1.000000 ))
\t\t\t(  real "Armor" (  ) ( data -1.000000 ))
\t\t) )'''

CARVERS = [('CarverDivLeft', (-500, -64, -16, 16, -16, 16)), ('CarverDivRight', (64, 500, -16, 16, -16, 16))]

def generic_obj(name, pos, rows):
    out = ['\t\t( proplist ( ',
           '\t\t\t(  string "Name" (  ) ( data "%s") ) ' % name,
           '\t\t\t(  vector "Pos" ( distance  ) ( data ( vector (%s) ) ) ) ' % ' '.join('%.6f' % v for v in pos),
           '\t\t\t(  rotation "Rotation" (  ) ( data (eulerangles (0.000000 0.000000 0.000000) ) ) ) ',
           '\t\t\t(  bool "Template" (  ) ( data 0 ))', '\t\t\t(  bool "IsGore" (  ) ( data 0 ))',
           '\t\t\t(  string "ObjectLOD" ( staticlist  ) ( data "Low") ) ']
    for kind, key, val in rows:
        if kind == 'string': out.append('\t\t\t(  string "%s" (  ) ( data "%s") ) ' % (key, val))
        elif kind == 'command': out.append('\t\t\t(  command "%s" ( notifychange  ) ( data "")) ' % key)
        elif kind == 'commandtext': out.append('\t\t\t(  command "%s" ( notifychange  ) ( data "%s")) ' % (key, val))
        elif kind == 'real': out.append('\t\t\t(  real "%s" (  ) ( data %.6f )) ' % (key, val))
        elif kind == 'vector': out.append('\t\t\t(  vector "%s" (  ) ( data ( vector (%s) ) ) ) ' % (key, ' '.join('%.6f' % v for v in val)))
        elif kind == 'longint': out.append('\t\t\t(  longint "%s" (  ) ( data %d.000000 )) ' % (key, val))
        else: out.append('\t\t\t(  bool "%s" (  ) ( data %d )) ' % (key, 1 if val else 0))
    out.append('\t\t) )')
    return '\n'.join(out)

def weapon_item(name, wtype, pos):
    return generic_obj(name, pos, [('real', 'Scale', 1.0), ('bool', 'StartHidden', 0), ('bool', 'Visible', 1), ('bool', 'Shadow', 1),
        ('bool', 'SPRespawn', 0), ('bool', 'MPRespawn', 1), ('bool', 'MoveToFloor', 1), ('bool', 'DropToFloor', 0),
        ('string', 'WeaponType', wtype), ('bool', 'GlobalResources', 0),
        ('string', 'Filename', r'global\weapons\%s\%s.Model00p' % (wtype.lower(), wtype.lower())), ('bool', 'Solid', 0), ('bool', 'Gravity', 0),
        ('real', 'LifeTime', -1.0), ('bool', 'TouchPickup', 1), ('bool', 'DMTouchPickup', 1), ('bool', 'Placed', 1), ('real', 'RespawnTime', 30.0)])

def prop_obj(name, model, filename, pos):
    return generic_obj(name, pos, [('string', 'Model', model), ('real', 'Scale', 1.0), ('bool', 'Visible', 1), ('bool', 'Solid', 0),
        ('bool', 'Gravity', 0), ('bool', 'Shadow', 1), ('bool', 'MoveToFloor', 1), ('bool', 'RigidBody', 0), ('real', 'Alpha', 1.0),
        ('bool', 'RayHit', 1), ('bool', 'GlobalResources', 0), ('real', 'HitPoints', 100.0), ('real', 'Armor', 100.0),
        ('bool', 'CanDamage', 1), ('string', 'Filename', filename)])


def trigger_obj(name, pos, dims, command):
    rows = [('vector', 'Dims', dims), ('longint', 'NumberOfActivations', 0), ('real', 'SendDelay', 0.0), ('real', 'TriggerDelay', 0.0),
            ('longint', 'Commands', 0), ('bool', 'PlayerTriggerable', 1), ('bool', 'AITriggerable', 0), ('bool', 'TimedTrigger', 0),
            ('real', 'MinTriggerTime', 0.0), ('real', 'MaxTriggerTime', 10.0), ('longint', 'ActivationCount', 1), ('bool', 'Locked', 0),
            ('real', 'SoundRadius', 200.0), ('real', 'HUDLookAtDist', -1.0), ('real', 'HUDAlwaysOnDist', -1.0), ('string', 'TriggerType', '<none>'),
            ('string', 'Team', 'NoTeam'), ('commandtext', 'Command1', command)]
    return generic_obj(name, pos, rows)

def door_obj(name, pos, locked=False):
    return generic_obj(name, pos, [('bool', 'Visible', 1), ('bool', 'Solid', 1), ('bool', 'Rayhit', 1), ('bool', 'Gravity', 0),
        ('bool', 'CastShadow', 1), ('string', 'PhysicsShape', 'OBB'), ('real', 'MassKg', 1.0), ('longint', 'Options', 0), ('longint', 'Sounds', 0),
        ('longint', 'Commands', 0), ('string', 'RotationPoint', ''), ('vector', 'RotationAngles', (0, 90, 0)), ('vector', 'StartingAngles', (0, 0, 0)),
        ('real', 'OpeningTime', 1.05), ('real', 'ClosingTime', 1.15), ('real', 'OpenWaitTime', 0.0), ('real', 'CloseWaitTime', 0.0),
        ('string', 'DoorLink', ''), ('string', 'ActivationType', 'Door'), ('real', 'Mass', 10000.0), ('bool', 'NeverDestroy', 1),
        ('bool', 'PlayerActivate', 1), ('bool', 'AIActivate', 1), ('bool', 'StartOpen', 0), ('bool', 'TriggerClose', 1), ('bool', 'RemainOn', 1),
        ('bool', 'RemainOpen', 1), ('bool', 'Locked', 1 if locked else 0), ('bool', 'RotateAway', 1), ('bool', 'OpenAway', 1), ('string', 'Waveform', 'Sine'),
        ('string', 'OpeningSound', 'door_mtl_opn_3'), ('string', 'ClosedSound', 'door_mtl_cls_3'), ('real', 'SoundRadius', 1000.0),
        ('longint', 'AWMType', 2), ('vector', 'MoveDir', (0, 0, 0)), ('real', 'MoveDist', 0.0), ('bool', 'GlobalResources', 0)])

DOOR_BOX = (-64, 64, 0, 220, -6, 6)

def build(stem, nav=False, ai=False, props=False, door=False, trigger=False, regions=False):
    brushes = [(n, b, m, 'Normal') for n, b, m in solid] + [(SECTOR[0], SECTOR[1], SECTOR[2], 'Sector')] + [(n, b, '', 'Sector') for n, b in EXTRA_SECTORS]
    if regions:  # two visibility sectors, one per streaming region (a sector must lie inside exactly one region)
        brushes = [b for b in brushes if b[3] != 'Sector'] + [('SectorA', (-500, 500, 0, H, -500, 0), '', 'Sector'), ('SectorB', (-500, 500, 0, H, 0, 500), '', 'Sector')]
    nbase = len(brushes)
    counts = {}
    for _, _, m, _ in brushes:
        if m: counts[m] = counts.get(m, 0) + 6
    dep = ''.join('\t\t(dependency \n\t\t\t(source "%%CurrentFile%%" )\n\t\t\t(resource "%s" )\n\t\t\t(resource-type "Material" )\n\t\t\t(type "Polygon" )\n\t\t\t(count %d )\n\t\t)\n' % (m, c) for m, c in counts.items())
    out = ['\t(resource-database \n\t(version 1 )\n\t(resource-list \n\t\t(resource \n\t\t\t(name "%CurrentFile%" )\n\t\t\t(type "World" )\n\t\t)\n\t)\n\t(dependency-list \n' + dep + '\t)\n)\n', '( world', '\t( header (', '\t\t( versioncode 2 )', '\t) )', '\t( polyhedronlist (']
    for _, b, m, _ in brushes:
        out.append(polyhedron(b, m))
    if nav:  # extra brushes: nav quad, two carvers, stalk child box (indices nbase..nbase+3)
        out.append(quad_polyhedron(-480, 480, 0, -480, 480))
        for _, bx in CARVERS:
            out.append(polyhedron(bx, ''))
        out.append(polyhedron((380, 440, 0, 40, -440, -380), ''))
    door_index = nbase + (4 if nav else 0)
    if door:
        out.append(polyhedron(DOOR_BOX, WALL))
    region_index = nbase + (4 if nav else 0) + (1 if door else 0)
    if regions:
        out.append(polyhedron((-540, 540, -40, 340, -540, 0.5), ''))
        out.append(polyhedron((-540, 540, -40, 340, 0.5, 540), ''))
    out.append('\t) )')
    # node hierarchy: root(propid 0), brushes, start point, lights
    nodes = []; plist = ['\t\t( proplist ( \n\t\t) )']; nid = 2
    for i, (n, b, m, t) in enumerate(brushes):
        plist.append(BRUSH_PROPS % (n + '%02d' % i, t))
        nodes.append('\t\t\t\t( worldnode \n\t\t\t\t\t( type brush )\n\t\t\t\t\t( brushindex %d )\n\t\t\t\t\t( nodeid %d ) \n\t\t\t\t\t( flags ( ) ) \n\t\t\t\t\t( properties\n\t\t\t\t\t\t( name "Brush" ) \n\t\t\t\t\t\t( propid  %d ) \n\t\t\t\t\t)\n\t\t\t\t)' % (i, nid, len(plist) - 1)); nid += 1
    objs = [('GameStartPoint', START_PROPS)]
    for k, pos in enumerate(('0.000000 200.000000 -270.000000', '0.000000 200.000000 270.000000')):
        objs.append(('LightPoint', LIGHT_PROPS % ('LightPoint%02d' % k, pos)))
    if ai:
        objs.append(('CAI', CAI_PROPS))
    if props:
        objs.append(('WeaponItem', weapon_item('Pipe00', 'Pipe', (-100, 30, -300))))
        objs.append(('WeaponItem', weapon_item('Plank00', '2x4', (100, 30, -300))))
        objs.append(('Prop', prop_obj('Crow00', 'crow', r'global\characters\animals\crow\crow.Model00p', (-250, 0, 300))))
    if trigger:
        objs.append(('Trigger', trigger_obj('TrigOpen00', (0, 60, -150), (200, 100, 60), 'msg CdmDoor00 unlock;msg CdmDoor00 on')))
    for cls, p in objs:
        plist.append(p)
        nodes.append('\t\t\t\t( worldnode \n\t\t\t\t\t( type object )\n\t\t\t\t\t( nodeid %d ) \n\t\t\t\t\t( flags ( ) ) \n\t\t\t\t\t( properties\n\t\t\t\t\t\t( name "%s" ) \n\t\t\t\t\t\t( propid  %d ) \n\t\t\t\t\t)\n\t\t\t\t)' % (nid, cls, len(plist) - 1)); nid += 1
    def brush_node(idx, pname, ptype):
        nonlocal nid
        plist.append(BRUSH_PROPS % (pname, ptype))
        s = '\t\t\t\t\t( worldnode \n\t\t\t\t\t\t( type brush )\n\t\t\t\t\t\t( brushindex %d )\n\t\t\t\t\t\t( nodeid %d ) \n\t\t\t\t\t\t( flags ( ) ) \n\t\t\t\t\t\t( properties\n\t\t\t\t\t\t\t( name "Brush" ) \n\t\t\t\t\t\t\t( propid  %d ) \n\t\t\t\t\t\t)\n\t\t\t\t\t)' % (idx, nid, len(plist) - 1)
        nid += 1
        return s
    def object_with_child(cls, prop_text, child_text):
        nonlocal nid
        plist.append(prop_text)
        s = '\t\t\t\t( worldnode \n\t\t\t\t\t( type object )\n\t\t\t\t\t( nodeid %d ) \n\t\t\t\t\t( flags ( ) ) \n\t\t\t\t\t( properties\n\t\t\t\t\t\t( name "%s" ) \n\t\t\t\t\t\t( propid  %d ) \n\t\t\t\t\t)\n\t\t\t\t\t( childlist ( \n%s\n\t\t\t\t\t) )\n\t\t\t\t)' % (nid, cls, len(plist) - 1, child_text)
        nid += 1
        return s
    if nav:
        nodes.append(object_with_child('AINavMesh', NAV_PROPS, brush_node(nbase, 'CdmNavQuad', 'Normal')))
        for k, (cn, _) in enumerate(CARVERS):
            nodes.append(brush_node(nbase + 1 + k, cn, 'AINavMeshCarver').replace('\n\t\t\t\t\t', '\n\t\t\t\t', 0))
        nodes.append(object_with_child('AINodeStalk', STALK_PROPS, brush_node(nbase + 3, 'StalkVolume', 'Normal')))
    if regions:
        for k, rn in enumerate(('RegionA', 'RegionB')):
            nodes.append(object_with_child('StreamingRegion', generic_obj(rn, (0, 0, 0), [('real', 'MemoryLimit', 25.0)]), brush_node(region_index + k, rn + '_Volume', 'Normal')))
        objs_info = generic_obj('StreamInfo', (0, 60, -300), [('real', 'GlobalLimit', 12.25), ('real', 'StreamingLimit', 83.5)])
        plist.append(objs_info)
        nodes.append('\t\t\t\t( worldnode \n\t\t\t\t\t( type object )\n\t\t\t\t\t( nodeid %d ) \n\t\t\t\t\t( flags ( ) ) \n\t\t\t\t\t( properties\n\t\t\t\t\t\t( name "StreamingInfo" ) \n\t\t\t\t\t\t( propid  %d ) \n\t\t\t\t\t)\n\t\t\t\t)' % (nid, len(plist) - 1)); nid += 1
    if door:
        nodes.append(object_with_child('RotatingDoor', door_obj('CdmDoor00', (-64, 0, 0), locked=trigger), brush_node(door_index, 'DoorPanel', 'Normal')))
    out.append('\t( nodehierarchy \n\t\t( worldnode \n\t\t\t( type null )\n\t\t\t( label "%s" ) \n\t\t\t( nodeid 1 ) \n\t\t\t( flags ( worldroot expanded ) ) \n\t\t\t( properties\n\t\t\t\t( propid  0 ) \n\t\t\t)\n\t\t\t( childlist ( \n%s\n\t\t\t) )\n\t\t)\n\t)' % (stem, '\n'.join(nodes)))
    out.append('\t( globalproplist (\n' + '\n'.join(plist) + '\n\t ) )\n\t( navigatorposlist (\n\t) )\n)\n')
    return '\n'.join(out)

if __name__ == '__main__':
    import pathlib
    p = pathlib.Path(sys.argv[1])
    p.write_text(build(p.stem, nav='nav' in sys.argv[2:], ai='ai' in sys.argv[2:], props='props' in sys.argv[2:], door='door' in sys.argv[2:], trigger='trigger' in sys.argv[2:], regions='regions' in sys.argv[2:]), encoding='ascii', newline='\n')
    print(p, p.stat().st_size)
