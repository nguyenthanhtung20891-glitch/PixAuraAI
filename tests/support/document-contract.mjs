// Engineering-only executable specification. Never imported by shipping code.
// Independent metadata oracle; production uses the shared native implementation.
import { Buffer } from 'node:buffer';

export const limits = Object.freeze({ bytes: 8 * 1024 * 1024, depth: 16, string: 256,
  operations: 4096, revisions: 4096, stack: 256, redo: 4095 });
const idPattern = /^[0-9a-f]{32}$/;
const digestPattern = /^[0-9a-f]{64}$/;
const fail = code => { throw new Error(code); };
const require = (condition, code = 'INVALID_PROJECT') => { if (!condition) fail(code); };
const id = value => require(typeof value === 'string' && idPattern.test(value));
const integer = (value, min, max, code = 'INVALID_PROJECT') =>
  require(Number.isSafeInteger(value) && !Object.is(value, -0) && value >= min && value <= max, code);
function keys(value, expected, code = 'INVALID_PROJECT') {
  require(value !== null && typeof value === 'object' && !Array.isArray(value), code);
  require(Object.keys(value).sort().join('|') === [...expected].sort().join('|'), code);
}
function array(value, max, min = 0) {
  require(Array.isArray(value));
  require(value.length <= max, 'RESOURCE_LIMIT');
  require(value.length >= min);
}
function unique(values) { require(new Set(values).size === values.length); }

export function validateOperation(operation) {
  keys(operation, ['id', 'type', 'operation_version', 'parameter_version', 'parameters']);
  id(operation.id);
  require(typeof operation.type === 'string' && /^[a-z][a-z0-9.]{0,63}$/.test(operation.type));
  integer(operation.operation_version, 1, 0xffffffff);
  integer(operation.parameter_version, 1, 0xffffffff);
  require(operation.operation_version === 1 && operation.parameter_version === 1, 'UNSUPPORTED_OPERATION');
  const parameter = operation.parameters;
  const check = (value, min, max) => integer(value, min, max, 'INVALID_PARAMETERS');
  switch (operation.type) {
    case 'pixaura.exposure':
      keys(parameter, ['milli_ev'], 'INVALID_PARAMETERS');
      check(parameter.milli_ev, -5000, 5000);
      break;
    case 'pixaura.rotate':
      keys(parameter, ['quarter_turns'], 'INVALID_PARAMETERS');
      check(parameter.quarter_turns, 0, 3);
      break;
    case 'pixaura.crop':
      keys(parameter, ['x_ppm', 'y_ppm', 'width_ppm', 'height_ppm'], 'INVALID_PARAMETERS');
      check(parameter.x_ppm, 0, 999999); check(parameter.y_ppm, 0, 999999);
      check(parameter.width_ppm, 1, 1000000); check(parameter.height_ppm, 1, 1000000);
      require(parameter.x_ppm + parameter.width_ppm <= 1000000 &&
        parameter.y_ppm + parameter.height_ppm <= 1000000, 'INVALID_PARAMETERS');
      break;
    default: {
      const contracts = {
        'pixaura.brightness': ['milli_linear', -1000, 1000],
        'pixaura.contrast': ['milli_stops', -2000, 2000],
        'pixaura.highlights': ['milli_ev', -2000, 2000],
        'pixaura.shadows': ['milli_ev', -2000, 2000],
        'pixaura.saturation': ['milli_ratio', 0, 2000],
        'pixaura.temperature': ['kelvin', 4000, 25000],
      };
      const contract = contracts[operation.type];
      require(contract !== undefined, 'UNSUPPORTED_OPERATION');
      keys(parameter, [contract[0]], 'INVALID_PARAMETERS');
      check(parameter[contract[0]], contract[1], contract[2]);
    }
  }
}

// Bounded token walk before JSON.parse: catches duplicates, depth, noninteger
// tokens and string limits that JSON.parse alone cannot enforce.
export function parseManifest(input) {
  const bytes = typeof input === 'string' ? Buffer.from(input) : input;
  require(bytes instanceof Uint8Array, 'INVALID_PROJECT');
  require(bytes.byteLength <= limits.bytes, 'RESOURCE_LIMIT');
  require(!(bytes[0] === 0xef && bytes[1] === 0xbb && bytes[2] === 0xbf));
  let text;
  try { text = new TextDecoder('utf-8', { fatal: true }).decode(bytes); }
  catch { fail('INVALID_PROJECT'); }
  let position = 0;
  const whitespace = () => { while (/[\t\r\n ]/.test(text[position] ?? '\0')) position++; };
  function string() {
    const start = position++;
    while (position < text.length) {
      const character = text[position++];
      if (character === '\\') { position++; continue; }
      if (character !== '"') continue;
      let value;
      try { value = JSON.parse(text.slice(start, position)); } catch { fail('INVALID_PROJECT'); }
      require(Buffer.byteLength(value) <= limits.string, 'RESOURCE_LIMIT');
      require(!/[\uD800-\uDBFF](?![\uDC00-\uDFFF])|(?<![\uD800-\uDBFF])[\uDC00-\uDFFF]/u.test(value));
      return value;
    }
    fail('INVALID_PROJECT');
  }
  function value(depth) {
    whitespace();
    const character = text[position];
    if (character === '{' || character === '[') {
      require(depth <= limits.depth, 'RESOURCE_LIMIT');
      position++;
      const end = character === '{' ? '}' : ']';
      const seen = new Set();
      whitespace();
      if (text[position] === end) { position++; return; }
      while (position < text.length) {
        if (character === '{') {
          require(text[position] === '"');
          const key = string();
          require(!seen.has(key)); seen.add(key);
          whitespace(); require(text[position++] === ':');
        }
        value(depth + 1); whitespace();
        if (text[position] === end) { position++; return; }
        require(text[position++] === ','); whitespace();
      }
      fail('INVALID_PROJECT');
    }
    if (character === '"') { string(); return; }
    const token = /^(?:true|false|null|-?(?:0|[1-9][0-9]*))/.exec(text.slice(position));
    require(token !== null);
    if (/^-?\d/.test(token[0])) integer(Number(token[0]), -Number.MAX_SAFE_INTEGER, Number.MAX_SAFE_INTEGER);
    position += token[0].length;
  }
  value(1); whitespace(); require(position === text.length);
  let result;
  try { result = JSON.parse(text); } catch { fail('INVALID_PROJECT'); }
  validateManifest(result);
  return result;
}

export function validateManifest(document) {
  keys(document, ['schema_version', 'project_id', 'document_id', 'source', 'operations',
    'revisions', 'current_revision_id', 'redo']);
  require(document.schema_version === 1, 'UNSUPPORTED_SCHEMA');
  id(document.project_id); id(document.document_id);
  keys(document.source, ['sha256', 'byte_length', 'metadata']);
  require(typeof document.source.sha256 === 'string' && digestPattern.test(document.source.sha256));
  integer(document.source.byte_length, 1, 8589934592);
  const metadata = document.source.metadata;
  keys(metadata, ['width', 'height', 'orientation', 'codec', 'has_alpha', 'icc_sha256']);
  integer(metadata.width, 1, 65535); integer(metadata.height, 1, 65535);
  require(metadata.width * metadata.height <= 268435456, 'RESOURCE_LIMIT');
  integer(metadata.orientation, 1, 8);
  require(['jpeg', 'png'].includes(metadata.codec) && typeof metadata.has_alpha === 'boolean');
  require(metadata.codec !== 'jpeg' || metadata.has_alpha === false);
  require(metadata.icc_sha256 === null || (typeof metadata.icc_sha256 === 'string' && digestPattern.test(metadata.icc_sha256)));
  array(document.operations, limits.operations);
  array(document.revisions, limits.revisions, 1);
  array(document.redo, limits.redo);
  const operations = new Map();
  for (const operation of document.operations) {
    validateOperation(operation); require(!operations.has(operation.id));
    operations.set(operation.id, operation);
  }
  const revisions = new Map(); const referenced = new Set();
  for (const [index, revision] of document.revisions.entries()) {
    keys(revision, ['id', 'parent_id', 'stack', 'actor', 'plan_id']);
    id(revision.id); require(!revisions.has(revision.id));
    array(revision.stack, limits.stack); unique(revision.stack);
    if (index === 0) {
      require(revision.parent_id === null && revision.actor === 'import' &&
        revision.plan_id === null && revision.stack.length === 0);
    } else {
      id(revision.parent_id); require(revisions.has(revision.parent_id));
      require(['manual', 'ai'].includes(revision.actor));
      if (revision.actor === 'ai') id(revision.plan_id);
      else require(revision.plan_id === null);
    }
    for (const operationId of revision.stack) {
      id(operationId); require(operations.has(operationId)); referenced.add(operationId);
    }
    revisions.set(revision.id, revision);
  }
  require(referenced.size === operations.size);
  id(document.current_revision_id); require(revisions.has(document.current_revision_id));
  unique(document.redo);
  let previous = document.current_revision_id;
  for (const revisionId of document.redo) {
    id(revisionId); require(revisions.get(revisionId)?.parent_id === previous);
    previous = revisionId;
  }
  return document;
}

function ordered(value) {
  if (Array.isArray(value)) return value.map(ordered);
  if (value !== null && typeof value === 'object') return Object.fromEntries(
    Object.keys(value).sort().map(key => [key, ordered(value[key])]));
  return value;
}
export function serialize(document) {
  validateManifest(document);
  const output = JSON.stringify(ordered(document)) + '\n';
  // Also enforces generic string/depth/byte limits for directly constructed values.
  parseManifest(output);
  return output;
}
export function replay(document, revisionId = document.current_revision_id) {
  validateManifest(document);
  const revision = document.revisions.find(value => value.id === revisionId);
  require(revision !== undefined);
  return revision.stack.map(operationId => structuredClone(document.operations.find(value => value.id === operationId)));
}

// Pure transition oracle; persistence and approval authorization are separate
// production application-service obligations. Caller supplies identity/generation.
export function transition(session, command) {
  validateManifest(session.document);
  id(session.session_id);
  integer(session.generation, 0, Number.MAX_SAFE_INTEGER);
  require(command.expected_session_id === session.session_id &&
    command.expected_revision_id === session.document.current_revision_id &&
    command.expected_generation === session.generation, 'STALE_BASE');
  require(session.generation < Number.MAX_SAFE_INTEGER, 'RESOURCE_LIMIT');
  const document = structuredClone(session.document);
  switch (command.kind) {
    case 'undo': {
      const current = document.revisions.find(value => value.id === document.current_revision_id);
      require(current.parent_id !== null, 'NO_HISTORY');
      document.redo.unshift(current.id); document.current_revision_id = current.parent_id;
      break;
    }
    case 'redo':
      require(document.redo.length > 0, 'NO_HISTORY');
      document.current_revision_id = document.redo.shift();
      break;
    case 'checkout':
      document.current_revision_id = command.revision_id; document.redo = [];
      break;
    case 'commit': {
      const existing = new Set(document.operations.map(value => value.id));
      for (const operation of command.operations) {
        require(!existing.has(operation.id)); existing.add(operation.id);
        require(command.stack.includes(operation.id));
      }
      document.operations.push(...structuredClone(command.operations));
      document.revisions.push({ id: command.revision_id, parent_id: document.current_revision_id,
        stack: structuredClone(command.stack), actor: command.actor, plan_id: command.plan_id });
      document.current_revision_id = command.revision_id; document.redo = [];
      break;
    }
    default: fail('INVALID_ARGUMENT');
  }
  serialize(document);
  return { document, session_id: session.session_id, generation: session.generation + 1 };
}
