/**
 * OpenGlt WebGL / WebAssembly Edition
 * 3D Model Visualizer with Dear ImGui styled controls, drag-and-drop,
 * scene hierarchy, object deletion, Shift+D duplicate, and mouse selection/movement.
 */

(function () {
    // --- State & Scene Graph ---
    let scene, camera, renderer, orbitControls, transformControls;
    let gridHelper, dirLight, spotLight, pointLight, lampMesh;
    let entities = [];
    let selectedEntity = null;
    let nextEntityId = 1;
    let isGrabMode = false;
    let grabStartPos = new THREE.Vector3();
    const raycaster = new THREE.Raycaster();
    const mouse = new THREE.Vector2();

    const canvasContainer = document.getElementById('canvas-container');
    const statusText = document.getElementById('status-text');
    const hierarchyList = document.getElementById('hierarchy-list');
    const entityCountBadge = document.getElementById('entity-count');

    // Inspector elements
    const inspectorEmpty = document.getElementById('inspector-empty');
    const inspectorDetails = document.getElementById('inspector-details');
    const propName = document.getElementById('prop-name');
    const propVisible = document.getElementById('prop-visible');
    const posX = document.getElementById('pos-x'), posY = document.getElementById('pos-y'), posZ = document.getElementById('pos-z');
    const rotX = document.getElementById('rot-x'), rotY = document.getElementById('rot-y'), rotZ = document.getElementById('rot-z');
    const scaleX = document.getElementById('scale-x'), scaleY = document.getElementById('scale-y'), scaleZ = document.getElementById('scale-z');

    function setStatus(msg) {
        if (statusText) statusText.textContent = msg;
    }

    // --- Init WebGL Engine ---
    function initEngine() {
        scene = new THREE.Scene();
        scene.background = new THREE.Color(0x131722);

        const width = window.innerWidth;
        const height = window.innerHeight;

        camera = new THREE.PerspectiveCamera(45, width / height, 0.1, 500);
        camera.position.set(0, 3, 8);

        renderer = new THREE.WebGLRenderer({ antialias: true, alpha: false });
        renderer.setSize(width, height);
        renderer.setPixelRatio(Math.min(window.devicePixelRatio, 2));
        renderer.shadowMap.enabled = true;
        renderer.shadowMap.type = THREE.PCFSoftShadowMap;
        canvasContainer.appendChild(renderer.domElement);

        // Orbit Controls (Right click to orbit, Middle click to pan, Scroll to zoom)
        orbitControls = new THREE.OrbitControls(camera, renderer.domElement);
        orbitControls.enableDamping = true;
        orbitControls.dampingFactor = 0.05;
        orbitControls.mouseButtons = {
            LEFT: null, // Reserved for object picking & gizmo
            MIDDLE: THREE.MOUSE.DOLLY,
            RIGHT: THREE.MOUSE.ROTATE
        };
        orbitControls.target.set(0, 0.5, 0);

        // Transform Controls (Gizmo)
        transformControls = new THREE.TransformControls(camera, renderer.domElement);
        transformControls.size = 0.85;
        transformControls.addEventListener('dragging-changed', (event) => {
            orbitControls.enabled = !event.value;
        });
        transformControls.addEventListener('change', () => {
            if (selectedEntity) {
                updateInspectorFromEntity(selectedEntity);
                updateSelectionWireframe(selectedEntity);
            }
        });
        scene.add(transformControls);

        // Grid & Environment
        gridHelper = new THREE.GridHelper(30, 30, 0x3b82f6, 0x222e44);
        gridHelper.position.y = -0.01;
        scene.add(gridHelper);

        // Lighting (matching OpenGlt C++ shader parameters)
        const ambientLight = new THREE.AmbientLight(0xffffff, 0.35);
        scene.add(ambientLight);

        dirLight = new THREE.DirectionalLight(0xffffff, 0.9);
        dirLight.position.set(5, 12, 7);
        dirLight.castShadow = true;
        dirLight.shadow.mapSize.width = 2048;
        dirLight.shadow.mapSize.height = 2048;
        scene.add(dirLight);

        spotLight = new THREE.SpotLight(0xffffff, 1.2, 40, Math.PI / 8, 0.3);
        spotLight.position.copy(camera.position);
        spotLight.target.position.set(0, 0, 0);
        scene.add(spotLight);
        scene.add(spotLight.target);

        // Point light lamp
        pointLight = new THREE.PointLight(0xffe6a3, 1.5, 20);
        pointLight.position.set(-2, 3, -1);
        scene.add(pointLight);

        const lampGeo = new THREE.BoxGeometry(0.25, 0.25, 0.25);
        const lampMat = new THREE.MeshBasicMaterial({ color: 0xfff0b3 });
        lampMesh = new THREE.Mesh(lampGeo, lampMat);
        lampMesh.position.copy(pointLight.position);
        scene.add(lampMesh);

        // Default Scene Objects
        addCubeEntity('Gold Cube', new THREE.Vector3(0, 0.5, 0), new THREE.Vector3(1, 1, 1));
        addSphereEntity('Sphere', new THREE.Vector3(2.5, 0.5, -0.5), new THREE.Vector3(1, 1, 1));

        // Window resize
        window.addEventListener('resize', onWindowResize);

        // Canvas Click for Raycast Selection
        renderer.domElement.addEventListener('pointerdown', onCanvasClick);

        // Start render loop
        animate();
    }

    function onWindowResize() {
        const width = window.innerWidth;
        const height = window.innerHeight;
        camera.aspect = width / height;
        camera.updateProjectionMatrix();
        renderer.setSize(width, height);
    }

    // --- Entity Management ---
    function addEntity(name, type, object3D, sourceFile = '') {
        const id = nextEntityId++;
        object3D.userData.entityId = id;

        // Traverse to mark all meshes
        object3D.traverse((child) => {
            if (child.isMesh) {
                child.castShadow = true;
                child.receiveShadow = true;
                child.userData.entityId = id;
            }
        });

        scene.add(object3D);

        const entity = {
            id,
            name: `${name} ${id}`,
            type,
            object3D,
            sourceFile,
            wireframeOutline: null
        };

        entities.push(entity);
        updateHierarchyUI();
        selectEntity(entity);
        setStatus(`Added ${entity.name}`);
        return entity;
    }

    function addCubeEntity(name = 'Gold Cube', pos = new THREE.Vector3(0, 0.5, 0), scale = new THREE.Vector3(1, 1, 1)) {
        const geo = new THREE.BoxGeometry(1, 1, 1);
        const mat = new THREE.MeshStandardMaterial({
            color: 0xd4af37, // Gold
            roughness: 0.3,
            metalness: 0.8
        });
        const mesh = new THREE.Mesh(geo, mat);
        mesh.position.copy(pos);
        mesh.scale.copy(scale);
        return addEntity(name, 'Cube', mesh);
    }

    function addSphereEntity(name = 'Sphere', pos = new THREE.Vector3(2, 0.5, 0), scale = new THREE.Vector3(1, 1, 1)) {
        const geo = new THREE.SphereGeometry(0.65, 32, 32);
        const mat = new THREE.MeshStandardMaterial({
            color: 0x38bdf8,
            roughness: 0.2,
            metalness: 0.6
        });
        const mesh = new THREE.Mesh(geo, mat);
        mesh.position.copy(pos);
        mesh.scale.copy(scale);
        return addEntity(name, 'Sphere', mesh);
    }

    // --- Selection & Wireframe Highlight ---
    function selectEntity(entity) {
        // Remove existing selection wireframe
        if (selectedEntity && selectedEntity.wireframeOutline) {
            scene.remove(selectedEntity.wireframeOutline);
            selectedEntity.wireframeOutline = null;
        }

        selectedEntity = entity;

        if (entity && entity.object3D) {
            transformControls.attach(entity.object3D);
            createSelectionWireframe(entity);
            updateInspectorFromEntity(entity);
            setStatus(`Selected: ${entity.name}`);
        } else {
            transformControls.detach();
            inspectorEmpty.style.display = 'block';
            inspectorDetails.style.display = 'none';
        }

        updateHierarchySelectionUI();
    }

    function createSelectionWireframe(entity) {
        const bbox = new THREE.Box3().setFromObject(entity.object3D);
        const helper = new THREE.Box3Helper(bbox, 0xf59e0b); // Golden Amber
        helper.material.linewidth = 2;
        entity.wireframeOutline = helper;
        scene.add(helper);
    }

    function updateSelectionWireframe(entity) {
        if (entity && entity.wireframeOutline) {
            entity.wireframeOutline.box.setFromObject(entity.object3D);
        }
    }

    // --- Mouse Click Raycast Selection ---
    function onCanvasClick(e) {
        // Only trigger on Left Click
        if (e.button !== 0) return;

        // Ignore if clicking on UI windows or dragging transform gizmo
        if (e.target !== renderer.domElement || transformControls.dragging) return;

        mouse.x = (e.clientX / window.innerWidth) * 2 - 1;
        mouse.y = -(e.clientY / window.innerHeight) * 2 + 1;

        raycaster.setFromCamera(mouse, camera);

        const meshes = [];
        entities.forEach(ent => {
            if (ent.object3D.visible) {
                ent.object3D.traverse(child => {
                    if (child.isMesh) meshes.push(child);
                });
            }
        });

        const intersects = raycaster.intersectObjects(meshes, false);

        if (intersects.length > 0) {
            const hitMesh = intersects[0].object;
            const hitId = hitMesh.userData.entityId;
            const hitEntity = entities.find(ent => ent.id === hitId);
            if (hitEntity) {
                selectEntity(hitEntity);
            }
        }
    }

    // --- Object Movement & Grab Mode ---
    function toggleGrabMode() {
        if (!selectedEntity) {
            setStatus('Select an object first to move');
            return;
        }
        isGrabMode = !isGrabMode;
        if (isGrabMode) {
            grabStartPos.copy(selectedEntity.object3D.position);
            setStatus(`Grab Mode: Move mouse to translate ${selectedEntity.name} (Click to confirm, Esc to cancel)`);
        } else {
            setStatus(`Moved ${selectedEntity.name}`);
        }
    }

    window.addEventListener('mousemove', (e) => {
        if (isGrabMode && selectedEntity) {
            const movementX = e.movementX || 0;
            const movementY = e.movementY || 0;
            const factor = camera.position.distanceTo(selectedEntity.object3D.position) * 0.002;

            const right = new THREE.Vector3(1, 0, 0).applyQuaternion(camera.quaternion);
            const up = new THREE.Vector3(0, 1, 0).applyQuaternion(camera.quaternion);

            selectedEntity.object3D.position.addScaledVector(right, movementX * factor);
            selectedEntity.object3D.position.addScaledVector(up, -movementY * factor);

            updateInspectorFromEntity(selectedEntity);
            updateSelectionWireframe(selectedEntity);
        }
    });

    window.addEventListener('click', () => {
        if (isGrabMode) {
            isGrabMode = false;
            setStatus(`Placed: ${selectedEntity.name}`);
        }
    });

    // --- Duplicate (Shift + D) ---
    function duplicateSelected() {
        if (!selectedEntity) {
            setStatus('No object selected to duplicate');
            return;
        }

        const cloneObj = selectedEntity.object3D.clone(true);
        // Offset slightly along X
        cloneObj.position.x += 1.2;

        const newEnt = addEntity(`${selectedEntity.name} (Copy)`, selectedEntity.type, cloneObj, selectedEntity.sourceFile);
        selectEntity(newEnt);
        setStatus(`Duplicated: ${newEnt.name} (Shift+D)`);
    }

    // --- Delete Object ---
    function deleteSelected() {
        if (!selectedEntity) {
            setStatus('No object selected to delete');
            return;
        }

        const name = selectedEntity.name;
        if (selectedEntity.wireframeOutline) {
            scene.remove(selectedEntity.wireframeOutline);
        }
        scene.remove(selectedEntity.object3D);

        const idx = entities.indexOf(selectedEntity);
        if (idx !== -1) {
            entities.splice(idx, 1);
        }

        transformControls.detach();
        const nextSel = entities.length > 0 ? entities[entities.length - 1] : null;
        selectEntity(nextSel);
        updateHierarchyUI();
        setStatus(`Deleted: ${name}`);
    }

    // --- Focus Camera on Entity ---
    function focusCameraOn(entity) {
        if (!entity || !entity.object3D) return;

        const bbox = new THREE.Box3().setFromObject(entity.object3D);
        const center = new THREE.Vector3();
        bbox.getCenter(center);
        const sphere = new THREE.Sphere();
        bbox.getBoundingSphere(sphere);
        const radius = Math.max(sphere.radius, 0.8);

        const offset = new THREE.Vector3(0, radius * 1.5, radius * 2.8);
        const targetPos = center.clone().add(offset);

        // Smooth camera transition
        const startPos = camera.position.clone();
        const startTarget = orbitControls.target.clone();
        let t = 0;

        function animateCam() {
            t += 0.05;
            camera.position.lerpVectors(startPos, targetPos, t);
            orbitControls.target.lerpVectors(startTarget, center, t);
            orbitControls.update();
            if (t < 1) {
                requestAnimationFrame(animateCam);
            }
        }
        animateCam();

        setStatus(`Focused camera on: ${entity.name}`);
    }

    // --- 3D Model Loading (GLTF / GLB / OBJ) ---
    function loadModelFromUrl(url, name = '3D Model') {
        setStatus(`Loading model: ${name}...`);
        const gltfLoader = new THREE.GLTFLoader();

        gltfLoader.load(
            url,
            (gltf) => {
                const model = gltf.scene;
                normalizeModelTransform(model);
                const ent = addEntity(name, '3D Model', model, url);
                focusCameraOn(ent);
                setStatus(`Successfully loaded: ${name}`);
            },
            (xhr) => {
                const pct = xhr.total ? Math.round((xhr.loaded / xhr.total) * 100) : '';
                if (pct) setStatus(`Loading ${name}... ${pct}%`);
            },
            (error) => {
                console.warn('GLTF load fallback, creating procedural proxy:', error);
                // Graceful fallback for local file CORS restrictions
                const fallbackGeo = new THREE.TorusKnotGeometry(0.8, 0.25, 64, 16);
                const fallbackMat = new THREE.MeshStandardMaterial({ color: 0x60a5fa, metalness: 0.8, roughness: 0.2 });
                const mesh = new THREE.Mesh(fallbackGeo, fallbackMat);
                mesh.position.set(0, 1, 0);
                const ent = addEntity(`${name}`, '3D Model', mesh);
                focusCameraOn(ent);
                setStatus(`Loaded model proxy: ${name}`);
            }
        );
    }

    function loadModelFromFile(file) {
        setStatus(`Reading ${file.name}...`);
        const reader = new FileReader();
        const ext = file.name.split('.').pop().toLowerCase();

        if (ext === 'gltf' || ext === 'glb') {
            reader.readAsArrayBuffer(file);
            reader.onload = (e) => {
                const gltfLoader = new THREE.GLTFLoader();
                gltfLoader.parse(
                    e.target.result,
                    '',
                    (gltf) => {
                        const model = gltf.scene;
                        normalizeModelTransform(model);
                        const ent = addEntity(file.name.replace(/\.[^/.]+$/, ''), '3D Model', model, file.name);
                        focusCameraOn(ent);
                        setStatus(`Imported & focused on: ${file.name}`);
                    },
                    (err) => {
                        setStatus(`Error parsing GLTF: ${err.message}`);
                    }
                );
            };
        } else if (ext === 'obj') {
            reader.readAsText(file);
            reader.onload = (e) => {
                const objLoader = new THREE.OBJLoader();
                const obj = objLoader.parse(e.target.result);
                normalizeModelTransform(obj);
                const ent = addEntity(file.name.replace(/\.[^/.]+$/, ''), 'OBJ Model', obj, file.name);
                focusCameraOn(ent);
                setStatus(`Imported & focused on: ${file.name}`);
            };
        } else {
            setStatus(`Unsupported file type: .${ext} (Use .gltf, .glb, .obj)`);
        }
    }

    function normalizeModelTransform(obj) {
        const bbox = new THREE.Box3().setFromObject(obj);
        const center = new THREE.Vector3();
        bbox.getCenter(center);
        obj.position.sub(center); // Center pivot

        const sphere = new THREE.Sphere();
        bbox.getBoundingSphere(sphere);
        if (sphere.radius > 5 || sphere.radius < 0.2) {
            const scale = 2.0 / Math.max(sphere.radius, 0.001);
            obj.scale.setScalar(scale);
        }
        obj.position.y += 0.5; // Rest on floor
    }

    // --- Drag and Drop File Handlers ---
    const dropOverlay = document.getElementById('drop-overlay');

    window.addEventListener('dragenter', (e) => {
        e.preventDefault();
        dropOverlay.classList.add('active');
    });

    window.addEventListener('dragover', (e) => {
        e.preventDefault();
        dropOverlay.classList.add('active');
    });

    window.addEventListener('dragleave', (e) => {
        if (e.relatedTarget === null || e.clientX <= 0 || e.clientY <= 0) {
            dropOverlay.classList.remove('active');
        }
    });

    window.addEventListener('drop', (e) => {
        e.preventDefault();
        dropOverlay.classList.remove('active');

        if (e.dataTransfer && e.dataTransfer.files.length > 0) {
            for (let i = 0; i < e.dataTransfer.files.length; i++) {
                loadModelFromFile(e.dataTransfer.files[i]);
            }
        }
    });

    // --- UI Synchronizations ---
    function updateHierarchyUI() {
        if (!hierarchyList) return;
        hierarchyList.innerHTML = '';
        entityCountBadge.textContent = `${entities.length} items`;

        entities.forEach((ent) => {
            const item = document.createElement('div');
            item.className = `hierarchy-item ${ent === selectedEntity ? 'selected' : ''}`;
            item.innerHTML = `
                <span><span class="item-badge">${ent.type}</span>${ent.name}</span>
                <span style="opacity: 0.6; font-size: 10px;">#${ent.id}</span>
            `;
            item.addEventListener('click', () => selectEntity(ent));
            hierarchyList.appendChild(item);
        });
    }

    function updateHierarchySelectionUI() {
        if (!hierarchyList) return;
        const items = hierarchyList.querySelectorAll('.hierarchy-item');
        entities.forEach((ent, i) => {
            if (items[i]) {
                if (ent === selectedEntity) {
                    items[i].classList.add('selected');
                } else {
                    items[i].classList.remove('selected');
                }
            }
        });
    }

    function updateInspectorFromEntity(ent) {
        if (!ent) return;
        inspectorEmpty.style.display = 'none';
        inspectorDetails.style.display = 'block';

        propName.value = ent.name;
        propVisible.checked = ent.object3D.visible;

        posX.value = ent.object3D.position.x.toFixed(2);
        posY.value = ent.object3D.position.y.toFixed(2);
        posZ.value = ent.object3D.position.z.toFixed(2);

        rotX.value = THREE.MathUtils.radToDeg(ent.object3D.rotation.x).toFixed(1);
        rotY.value = THREE.MathUtils.radToDeg(ent.object3D.rotation.y).toFixed(1);
        rotZ.value = THREE.MathUtils.radToDeg(ent.object3D.rotation.z).toFixed(1);

        scaleX.value = ent.object3D.scale.x.toFixed(2);
        scaleY.value = ent.object3D.scale.y.toFixed(2);
        scaleZ.value = ent.object3D.scale.z.toFixed(2);
    }

    // Inspector input listeners
    propName.addEventListener('input', (e) => {
        if (selectedEntity) {
            selectedEntity.name = e.target.value;
            updateHierarchyUI();
        }
    });

    propVisible.addEventListener('change', (e) => {
        if (selectedEntity) {
            selectedEntity.object3D.visible = e.target.checked;
            if (selectedEntity.wireframeOutline) {
                selectedEntity.wireframeOutline.visible = e.target.checked;
            }
        }
    });

    function onTransformInput() {
        if (!selectedEntity) return;
        selectedEntity.object3D.position.set(parseFloat(posX.value) || 0, parseFloat(posY.value) || 0, parseFloat(posZ.value) || 0);
        selectedEntity.object3D.rotation.set(
            THREE.MathUtils.degToRad(parseFloat(rotX.value) || 0),
            THREE.MathUtils.degToRad(parseFloat(rotY.value) || 0),
            THREE.MathUtils.degToRad(parseFloat(rotZ.value) || 0)
        );
        selectedEntity.object3D.scale.set(parseFloat(scaleX.value) || 1, parseFloat(scaleY.value) || 1, parseFloat(scaleZ.value) || 1);
        updateSelectionWireframe(selectedEntity);
    }

    [posX, posY, posZ, rotX, rotY, rotZ, scaleX, scaleY, scaleZ].forEach(inp => {
        inp.addEventListener('input', onTransformInput);
    });

    // --- Event Listeners & Hotkeys ---
    document.getElementById('btn-add-cube').addEventListener('click', () => addCubeEntity());
    document.getElementById('btn-add-sphere').addEventListener('click', () => addSphereEntity());
    document.getElementById('btn-dup-selected').addEventListener('click', duplicateSelected);
    document.getElementById('btn-del-selected').addEventListener('click', deleteSelected);
    document.getElementById('btn-focus-selected').addEventListener('click', () => focusCameraOn(selectedEntity));
    document.getElementById('btn-insp-grab').addEventListener('click', toggleGrabMode);

    // Toggle Grid
    const btnToggleGrid = document.getElementById('btn-toggle-grid');
    btnToggleGrid.addEventListener('click', () => {
        gridHelper.visible = !gridHelper.visible;
        btnToggleGrid.classList.toggle('active', gridHelper.visible);
    });

    // Toggle Spotlight
    const btnToggleLight = document.getElementById('btn-toggle-light');
    btnToggleLight.addEventListener('click', () => {
        spotLight.visible = !spotLight.visible;
        btnToggleLight.classList.toggle('active', spotLight.visible);
    });

    // Gizmo Mode buttons
    const gizmoT = document.getElementById('gizmo-translate');
    const gizmoR = document.getElementById('gizmo-rotate');
    const gizmoS = document.getElementById('gizmo-scale');

    function setGizmoMode(mode) {
        transformControls.setMode(mode);
        gizmoT.classList.toggle('active', mode === 'translate');
        gizmoR.classList.toggle('active', mode === 'rotate');
        gizmoS.classList.toggle('active', mode === 'scale');
    }

    gizmoT.addEventListener('click', () => setGizmoMode('translate'));
    gizmoR.addEventListener('click', () => setGizmoMode('rotate'));
    gizmoS.addEventListener('click', () => setGizmoMode('scale'));

    // File input handlers
    const fileInput = document.getElementById('file-input');
    const fileInputBrowser = document.getElementById('file-input-browser');

    fileInput.addEventListener('change', (e) => {
        if (e.target.files.length > 0) loadModelFromFile(e.target.files[0]);
    });
    fileInputBrowser.addEventListener('change', (e) => {
        if (e.target.files.length > 0) loadModelFromFile(e.target.files[0]);
    });

    // Bundled Model Load buttons in Browser panel
    document.querySelectorAll('.file-load-btn').forEach((btn) => {
        btn.addEventListener('click', (e) => {
            const entry = e.target.closest('.file-entry');
            const path = entry.getAttribute('data-path');
            const name = entry.getAttribute('data-name');
            loadModelFromUrl(path, name);
        });
    });

    // Keyboard Shortcuts
    window.addEventListener('keydown', (e) => {
        // Ignore if typing in input
        if (e.target.tagName === 'INPUT') return;

        // Shift + D: Duplicate
        if (e.shiftKey && (e.key === 'D' || e.key === 'd')) {
            e.preventDefault();
            duplicateSelected();
        }
        // Delete / Backspace: Delete
        else if (e.key === 'Delete' || e.key === 'Backspace') {
            e.preventDefault();
            deleteSelected();
        }
        // F: Focus camera
        else if (e.key === 'f' || e.key === 'F') {
            e.preventDefault();
            focusCameraOn(selectedEntity);
        }
        // G: Grab / Move
        else if (e.key === 'g' || e.key === 'G') {
            e.preventDefault();
            toggleGrabMode();
        }
        // W: Translate mode
        else if (e.key === 'w' || e.key === 'W') {
            setGizmoMode('translate');
        }
        // E: Rotate mode
        else if (e.key === 'e' || e.key === 'E') {
            setGizmoMode('rotate');
        }
        // R: Scale mode
        else if (e.key === 'r' || e.key === 'R') {
            setGizmoMode('scale');
        }
        // T: Toggle Spotlight
        else if (e.key === 't' || e.key === 'T') {
            btnToggleLight.click();
        }
        // Escape: Cancel grab mode or deselect
        else if (e.key === 'Escape') {
            if (isGrabMode && selectedEntity) {
                selectedEntity.object3D.position.copy(grabStartPos);
                isGrabMode = false;
                updateInspectorFromEntity(selectedEntity);
                updateSelectionWireframe(selectedEntity);
                setStatus('Move cancelled');
            }
        }
    });

    // --- Render Loop ---
    function animate() {
        requestAnimationFrame(animate);

        orbitControls.update();

        // Spotlight tracks camera position and direction
        if (spotLight && spotLight.visible) {
            spotLight.position.copy(camera.position);
            const dir = new THREE.Vector3();
            camera.getWorldDirection(dir);
            spotLight.target.position.copy(camera.position).add(dir.multiplyScalar(10));
        }

        renderer.render(scene, camera);
    }

    // Run engine on load
    window.addEventListener('DOMContentLoaded', initEngine);
})();
