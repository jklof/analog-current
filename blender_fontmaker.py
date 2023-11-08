import bpy


# (load) and select font
font_name = 'Bauhaus 93 Regular'  # font name in blender
font_path = 'C:/WINDOWS/Fonts/BAUHS93.TTF' # path to font file
font_scale = 512  # fixed point scaling
font_size = 0.7 # overall font size( scale) in blender

if font_name not in bpy.data.fonts:
    # Load the font from the specified file
    font_data = bpy.data.fonts.load(font_path)
else:
    print("font is loaded")

font = bpy.data.fonts[font_name]


# letters that are generated
alphabet = '.+-:=/#*0123456789ABCDEFGHIJKLMNOPQRSTUWVXYZ$'

# some letters have special names
def lname(l):
    names = {
        '.' : '_dot',
        '+' : '_plus',
        '-' : '_minus',
        ':' : '_colon',
        '=' : '_equals',
        '/' : '_slash',
        '#' : '_hash',
        '*' : '_asterisk',
        '$' : '_kwh',
    }
    return 'Letter_' + names.get(l,l)

# some letters have special string values
def lvalue(c):
    value = {
        '$' : 'c€/kWh' # replace with
    }    
    return value.get(c,c)


# some letters have different size from font_size
def lsize(c):
    size = {
        '$' : 0.35
    }
    return size.get(c,font_size)


def create_letters(fill_mode='FRONT'):
    for a in list(alphabet):
        bpy.ops.object.text_add()
        o = bpy.context.object
        o.name = lname(a)
        o.data.body = lvalue(a)
        o.data.font = font
        o.data.resolution_u = 2
        o.data.fill_mode = fill_mode
        o.data.size = lsize(a)

def select_letters():
    bpy.ops.object.select_all(action='DESELECT')
    for a in list(alphabet):
        name = lname(a)
        if name in bpy.data.objects:
            bpy.data.objects[name].select_set(True)

def delete_letters():
    select_letters()
    bpy.ops.object.delete()


def convert_to_mesh():
    select_letters()
    bpy.ops.object.convert(target='MESH')


def beauty_fill():
    select_letters()
    bpy.ops.object.mode_set(mode='EDIT')  # Enter Edit mode
    bpy.ops.mesh.select_all(action='SELECT')  # Select all vertices
    bpy.ops.mesh.beautify_fill() 
    bpy.ops.object.mode_set(mode='OBJECT')  # Exit Edit mode


def check():  
    for a in list(alphabet):
        name = lname(a)
        bpy.ops.object.select_all(action='DESELECT')
        bpy.data.objects[name].select_set(True)
        bpy.ops.object.mode_set(mode='EDIT')
        
        has_ngons = False
        for polygon in bpy.context.object.data.polygons:
            if len(polygon.vertices) > 4:
                has_ngons = True
                break
        bpy.ops.object.mode_set(mode='OBJECT')
        if has_ngons:
            print(f"The object '{name}' contains ngons.")


# converts from 3d to fixed 2d coordinate space
def to_2d_fixed_point_vert(vertex):
    scale = font_scale
    return (  int( vertex.co.x * scale) , int(vertex.co.y * scale) )


# all vertices as fixed point and make sure they're unique
def get_all_verts_as_fixed_point():
    verts = []
    for a in list(alphabet):
        name = lname(a)
        mesh = bpy.data.objects[name].data
        for vertex in mesh.vertices:
            co = to_2d_fixed_point_vert(vertex)
            if co not in verts:
                verts.append(co)
    return verts


def save_lines_as_c_header(filename):

    with open(filename, 'w') as f:

        # collect all vertices to the same buffer
        verts = get_all_verts_as_fixed_point()

        # write all vertices
        f.write(f'#define  LINE_FONT_SCALE {font_scale}\n')
        f.write('const int16_t line_font_vertices[] PROGMEM = {\n')
        ii = 0
        for x,y in verts:
            f.write(f'  {x}, {y} /*{ii}*/,\n')
            ii += 1
        f.write('};\n')

        # now write edges (lines) for each letter
        advance = {} # avance for each letter..
        for a in list(alphabet):
            name = lname(a)
            mesh = bpy.data.objects[name].data
            adv = 0

            f.write(f'const int16_t {name}_lines[] PROGMEM = {{\n')
            for edge in mesh.edges:
                for vi in edge.vertices:
                    # convert value to 2d integer space
                    x,y = to_2d_fixed_point_vert(mesh.vertices[vi])
                    adv = max(adv,x)
                    # look for matching int in all verts
                    verts_index = verts.index((x,y))
                    f.write(f'  {verts_index} /*{x},{y}*/,')
                f.write('\n')
            f.write('};\n')
            advance[name] = adv
                

        f.write('const struct {\n')
        f.write(' const char chr;\n')
        f.write(' const int16_t advance;\n')
        f.write(' const int16_t *lines;\n')
        f.write(' const int16_t size;\n')
        f.write('} font_line_geometry[] = {\n')

        for a in list(alphabet):
            name = lname(a)
            adv = advance[name]
            f.write(f' {{\'{a}\', {adv}, {name}_lines, sizeof({name}_lines) / sizeof({name}_lines[0]) }},\n')
        f.write('};\n')




def save_polygons_as_c_header(filename):

    with open(filename, 'w') as f:

        # collate all vertices to the same buffer
        verts = get_all_verts_as_fixed_point()

        # write all vertices
        f.write(f'#define  TRI_FONT_SCALE {font_scale}\n')
        f.write('const int16_t tri_font_vertices[] PROGMEM = {\n')
        ii = 0
        for x,y in verts:
            f.write(f'  {x}, {y} /*{ii}*/,\n')
            ii += 1
        f.write('};\n')

        # now write polygons (triangles) for each letter
        advance = {} # avance for each letter..
        for a in list(alphabet):
            name = lname(a)
            mesh = bpy.data.objects[name].data
            adv = 0

            f.write(f'const int16_t {name}_tris[] PROGMEM = {{\n')
            for face in mesh.polygons:
                for vi in face.vertices:
                    # convert value to 2d integer space
                    x,y = to_2d_fixed_point_vert(mesh.vertices[vi])
                    adv = max(adv,x)
                    # look for matching int in all verts
                    verts_index = verts.index((x,y))
                    f.write(f'  {verts_index} /*{x},{y}*/,')
                f.write('\n')
            f.write('};\n')
            advance[name] = adv

        f.write('const struct {\n')
        f.write(' const char chr;\n')
        f.write(' const int16_t advance;\n')
        f.write(' const int16_t *tris;\n')
        f.write(' const int16_t size;\n')
        f.write('} font_tri_geometry[] = {\n')

        for a in list(alphabet):
            name = lname(a)
            adv = advance[name]
            f.write(f' {{\'{a}\', {adv}, {name}_tris, sizeof({name}_tris) / sizeof({name}_tris[0]) }},\n')
        f.write('};\n')


#export as wavefront
def export(filename):
    select_letters()
    bpy.ops.wm.obj_export(filepath=filename, export_materials=False, export_selected_objects=True)

# triangle letters
delete_letters()
create_letters(fill_mode='FRONT')
convert_to_mesh()
beauty_fill()
save_polygons_as_c_header("./tft_display/bauhaus_tris.h")

# outline edge letters
delete_letters()
create_letters(fill_mode='NONE')
convert_to_mesh()
save_lines_as_c_header("./tft_display/bauhaus_lines.h")
export("./vfont.obj")
