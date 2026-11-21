import { defineConfig } from 'vite';
import react from '@vitejs/plugin-react';
import path from 'node:path';

// Vite 6 配置：@/ 别名指向 src/
export default defineConfig({
  plugins: [react()],
  resolve: {
    alias: {
      '@': path.resolve(__dirname, './src'),
    },
  },
  server: {
    port: 9003,
    host: true,
    // 开发期同源代理：把后端路由前缀转发到 9000，避免浏览器 CORS 拦截
    proxy: {
      "^/(login|register|captcha|user|friend|message|file|transmite)/": {
        target: "http://127.0.0.1:9000",
        changeOrigin: true,
      },
    },
  },
});
