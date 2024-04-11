import xml.etree.ElementTree as ET
import struct


ns = ''

def readFloatArray(el:ET.Element):
    array = {}
    array['id'] = el.get('id')
    array['count'] = int(el.get('count'))
    array['data'] = [float(item) for item in  el.text.split(' ')]
    return array

def readParam(el:ET.Element):
    param = {}
    param['name'] = el.get('name')
    param['type'] = el.get('type')
    return param

def readAccessor(el:ET.Element):
    accessor = {}
    accessor['source'] = el.get('source')
    accessor['count']  = int(el.get('count'))
    accessor['stride'] = int(el.get('stride'))
    accessor['params'] = [readParam(item) for item in  el.findall(ns + 'param')]
    return accessor

def readTechniqueCommon(el:ET.Element):
    technique = {}
    technique['accessor'] = readAccessor(el.find(ns + 'accessor'))
    return technique

def readSource(el: ET.Element):
    source = {}
    source['id'] = el.get('id')
    for child in el:
        if child.tag == ns + 'float_array':
            source['array'] = readFloatArray(child)
        elif child.tag == ns + 'technique_common':
            source['technique_common'] = readTechniqueCommon(child)
    return source

def readVertex(el: ET.Element):
    vertex = {}
    inputs = []
    for inputElement in el.findall(ns + 'input'):
        input = {}
        input['semantic'] = inputElement.get('semantic')
        input['source'] = inputElement.get('source')
        inputs.append(input)
    vertex['inputs'] = inputs
    return vertex

def readTriangles(el: ET.Element):
    triangles = {}
    inputs = []

    triangles['material'] = el.get('material')
    triangles['count'] = int(el.get('count'))

    for inputElement in el.findall(ns + 'input'):
        input = {}
        input['semantic'] = inputElement.get('semantic')
        input['source']   = inputElement.get('source')
        input['offset']   = int(inputElement.get('offset'))
        # input['set']      = int(inputElement.get('set'))
        inputs.append(input)
    triangles['inputs'] = inputs

    triangles['data'] = list(map(int, [int(item, 10) for item in  el.find(ns + 'p').text.split(' ')]))
    return triangles

def readMesh(el:ET.Element):
    mesh    = {}
    sources = []
    vertices = []
    triangles = []
    for source in el.findall(ns + 'source'):
        sources.append(readSource(source))
    mesh['sources'] = sources

    for vertex in el.findall(ns + 'vertices'):
        vertices.append(readVertex(vertex))
    mesh['vertices'] = vertices

    for triangle in el.findall(ns + 'triangles'):
        triangles.append(readTriangles(triangle))
    mesh['triangles'] = triangles

    return mesh


def readGeometry(el: ET.Element):
    geometry = {}
    meshes = []
    for mesh in el:
        meshes.append(readMesh(mesh))
    geometry['meshes'] = meshes
    return geometry

def read_xml(file_path):
    global ns
    G = {}
    try:
        # Parse the XML file
        tree = ET.parse(file_path)
        root = tree.getroot()
        namespace_uri, _, namespace_prefix = root.tag.partition('}')
        ns = namespace_uri + '}'
        
        for child in root:
            if child.tag == ns + 'library_geometries':
                for geometry in child:
                    G = readGeometry(geometry)
    except Exception as e:
        print("Error:", e)

    return G

def writeFile(geometry: dict):
    # Writing floats to a binary file
    with open('float_data.bin', 'wb') as f:
        f.write(struct.pack('i', model['nVertices']))  # Write the count of floats as an integer
        f.write(struct.pack('i', model['nIndices']))  # Write the count of floats as an integer
        for vertex in model['vertices']:
            f.write(struct.pack('%sf' % len(vertex), *vertex))
        f.write(struct.pack('%si' % len(model['indices']), *model['indices']))


def geometryToModel(geometry: dict)->dict:
    model = {}
    for mesh in geometry['meshes']:
        for source in mesh['sources']:
            count = source['technique_common']['accessor']['count']
            stride = source['technique_common']['accessor']['stride']
            data = source['array']['data']
            if 'normals' in source['id']:
                model['nNormals'] = count
                model['normals'] = [data[idx*stride:idx*stride + stride:] for idx in range(count)]
            elif 'positions' in source['id']:
                model['nPositions'] = count
                model['positions'] = [data[idx*stride:idx*stride + stride:] for idx in range(count)]
            elif 'map-0' in source['id']:
                model['nTexels'] = count
                model['texels'] = [data[idx*stride:idx*stride + stride:] for idx in range(count)]

            for materialGroup in mesh['triangles']:
                count = materialGroup['count']
                stride = len(materialGroup['inputs'])
                data = materialGroup['data']
                model['nTriangles'] = count  # Write the count of floats as an integer
                model['triangles'] = [data[triangleIdx*3*stride + vertexIdx*stride:triangleIdx*3*stride + vertexIdx*stride + stride:] for triangleIdx in range(count) for vertexIdx in range(3)]
    
    mapIndices = [[[-1 for iTexel in range(model['nTexels'])] for iNormal in range(model['nNormals'])] for iPosition in range(model['nPositions'])]
    outIndices = []
    outVertices = []
    for idxPosition, idxNormal, idxTexel in model['triangles']:
        if mapIndices[idxPosition][idxNormal][idxTexel] == -1:
            # create new index
            newVertex = []
            newVertex.extend(model['positions'][idxPosition])
            newVertex.extend(model['normals'][idxNormal])
            newVertex.extend(model['texels'][idxTexel])
            # newVertex.extend(model['texels'][idxTexel])
            outVertices.append(newVertex)
            mapIndices[idxPosition][idxNormal] = len(outVertices) - 1
        outIndices.append(mapIndices[idxPosition][idxNormal])
    
    outModel = {}
    outModel['nVertices'] = len(outVertices)
    outModel['vertices'] = outVertices
    outModel['nIndices'] = len(outIndices)
    outModel['indices'] = outIndices
    return outModel

# Example usage
if __name__ == "__main__":
    file_path = "sphere.dae"
    geometry = read_xml(file_path)
    model = geometryToModel(geometry)
    writeFile(model)
