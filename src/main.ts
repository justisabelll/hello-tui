import { createRequire } from 'module';
import {
  createCliRenderer,
  Box,
  Text,
  TextRenderable,
} from '@opentui/core';

const require = createRequire(import.meta.url);

const cpp = require('../build/Release/hello.node') as {
  getGreeting: (name: string) => string;
  tickFrame: () => number;
  resetFrames: () => void;
};

if (
  typeof cpp.getGreeting !== 'function' ||
  typeof cpp.tickFrame !== 'function' ||
  typeof cpp.resetFrames !== 'function'
) {
  throw new Error('addon loaded but one or more expected exports are missing');
}

const renderer = await createCliRenderer({
  exitOnCtrlC: true,
  targetFps: 30,
});

process.on('SIGTERM', () => {
  renderer.destroy();
  process.exit(0);
});

const greetingText = new TextRenderable(renderer, {
  id: 'greeting',
  content: '',
  fg: '#9ECE6A',
});

const counterText = new TextRenderable(renderer, {
  id: 'counter',
  content: '',
  fg: '#E0AF68',
});

renderer.root.add(
  Box(
    {
      width: '100%',
      height: '100%',
      flexDirection: 'column',
      gap: 1,
      padding: 2,
    },

    // static header
    Text({
      content: 'Bun Node-API + Cpp + OpenTUI Demo',
      fg: '#7DCFFF',
    }),

    greetingText,
    counterText,
    Text({
      content: 'Press Ctrl+C to exit  |  Press R to reset counter',
      fg: '#565F89',
    }),
  ),
);

const greeting = cpp.getGreeting('World');
greetingText.content = greeting;

const loopInterval = setInterval(() => {
  const frame = cpp.tickFrame();
  counterText.content = `Native frame count: ${frame}`;
}, 100);

renderer.addInputHandler((sequence) => {
  if (sequence === 'r' || sequence === 'R') {
    cpp.resetFrames();
    counterText.content = `Native frame count: ${cpp.tickFrame()}`;
    return true; // consume the input
  }
  return false; // let other handlers process
});

renderer.on('destroy', () => {
  clearInterval(loopInterval);
});
